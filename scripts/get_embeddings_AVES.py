
import torch
import torchaudio
from torch.utils.data import Dataset, DataLoader
import torch.nn.functional as F
from torchaudio.models import wav2vec2_model

import json
import torch.nn as nn
import sklearn.cluster
from tqdm import tqdm
import matplotlib.pyplot as plt
# import ipympl
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
from sklearn.manifold import TSNE

import umap
import numpy as np
from sklearn.preprocessing import LabelEncoder
import pandas as pd




# Class definitions from AVES
class Vox(Dataset):
    def __init__(self, dataset_dataframe, audio_sr, annotation_names):
        """ Dataset for vocalization classification with AVES

        Input
        -----
        dataset_dataframe (pandas dataframe): indicating the filepath, annotations and partition of a signal
        audio_sr (int): sampling rate expected by network
        annotation_name (list[str]): string corresponding to the annotation columns in the dataframe, e.g. ["type","recording_date"]
        """
        super().__init__()
        self.audio_sr = audio_sr
        self.annotation_names = annotation_names
        self.dataset_info = dataset_dataframe

    def __len__(self):
        return len(self.dataset_info)

    def get_one_item(self, idx):
      """ Load base audio """
      row = self.dataset_info.iloc[idx]
      x, sr = torchaudio.load(row["filepath"])
      if len(x.size()) == 2:
          x = x[0, :]
      if sr != self.audio_sr:
          x = torchaudio.functional.resample(x, sr, self.audio_sr)
      return x, row

    def __getitem__(self, idx):
        x, row = self.get_one_item(idx)
        out = {"x" : x, "filepath" : row['filepath'], "filename" : row['filename']}
        for k in self.annotation_names:
          out[k] = row[k]
        return out

def get_dataloader(dataset_dataframe, audio_sr, annotation_names):
    return DataLoader(
            Vox(dataset_dataframe, audio_sr, annotation_names),
            batch_size=1,
            shuffle=False,
            drop_last=False)

class AvesMeanEmbedding(nn.Module):
    """ Uses AVES Hubert to embed sounds and classify """
    def __init__(self, config_path, model_path, embedding_dim=768):
        super().__init__()
        # reference: https://pytorch.org/audio/stable/_modules/torchaudio/models/wav2vec2/utils/import_fairseq.html
        self.config = self.load_config(config_path)
        self.model = wav2vec2_model(**self.config, aux_num_out=None)
        self.model.load_state_dict(torch.load(model_path))
        self.audio_sr = 16000 # this can be tuned

    def load_config(self, config_path):
        with open(config_path, 'r') as ff:
            obj = json.load(ff)
        return obj

    def forward(self, sig):
        """
        Input
          sig (Tensor): (batch, time)
        Returns
          mean_embedding (Tensor): (batch, output_dim)
          logits (Tensor): (batch, n_classes)
        """
        # extract_feature in the sorchaudio version will output all 12 layers' output, -1 to select the final one
        out = self.model.extract_features(sig)[0][-1]
        mean_embedding = out.mean(dim=1) #over time
        return mean_embedding

def run_model(input_dataframe, metadata_column_array):
  print("~~ Setting up model")
  embedding_model = AvesMeanEmbedding(
      config_path="./AVES_finetuning/aves_base_bio_files/aves-base-bio.torchaudio.model_config.json",
      model_path="./AVES_finetuning/aves_base_bio_files/aves-base-bio.torchaudio.pt"
    );

  # check model structure
  embedding_model.eval()

  # For GPU-based parallelization
  if torch.cuda.is_available(): 
    embedding_model.cuda()

  
  print("~~ Setting up dataloader")
  dataloader = get_dataloader(dataset_dataframe=input_dataframe, # RCG: this is a df with:  filename, filepath, individual, recording_date, type
                              audio_sr=embedding_model.audio_sr, # RCG: Audio sampling rate
                              annotation_names=metadata_column_array ) # RCG:   annotation_names in dataset_dataframe "type", "individual", "recording_date"
  # dataloader

  print(f"~~ Compute AVES embeddings")

  annotation_names=metadata_column_array
  with torch.no_grad():
    features = []; filepaths = []; filenames = []; known_classes = {k:[] for k in annotation_names}
    for data_idx, data_dict in enumerate(tqdm(dataloader)):
        x = data_dict["x"]
        if torch.cuda.is_available():
          x = x.cuda()
        features.append(embedding_model(x).cpu().numpy())
        filepaths.append(data_dict["filepath"])
        filenames.append(data_dict["filename"])



        for k in annotation_names:
          known_classes[k].append(data_dict[k])
      
        tmp_X = np.concatenate(features, axis=0)
        header = ','.join([f"feature_{i}" for i in range(tmp_X.shape[1])])
        np.savetxt("./data/euro_cities_recordings/tmp_embeddings.csv", tmp_X, delimiter=",", header=header)

        # with open('./data/euro_cities_recordings/datadict.json', 'w') as json_file:
        #   json.dump(data_dict, json_file)
  
  print(f"~~ Collect latent space")
  X = np.concatenate(features, axis=0) #(n_samples, n_features)
  X.shape
  
  return X


# plotting functions
def PCAplot(embedding_df):
  """
  Input is usually the X object from AVES
  """
   
  mat = np.array(embedding_df)
  mat.shape

  # Scale data # DOUBT: do I have to scale data from a model or is it already scaled ????
  scaler = StandardScaler()
  scaler.fit(mat) 
  mat_scaled = scaler.transform(mat)

  pca = PCA(n_components=3)
  pca.fit(mat_scaled) 
  X_pca = pca.transform(mat_scaled)

  df = pd.DataFrame()
  df['PC1'] = X_pca[:,0]
  df['PC2'] = X_pca[:,1]
  df['PC3'] = X_pca[:,2]

  print('Explained variation per principal component: {}'.format(pca.explained_variance_ratio_))


  # Use hot-encodings
  label_encoder = LabelEncoder()
  df['Color'] = label_encoder.fit_transform(df['type'])
  df['type'].unique()


  # Then plot
  fig = plt.figure(figsize=(10,10))
  ax = fig.add_subplot(111, projection='3d')
  # Pallette
  # cmap = ListedColormap(sns.color_palette("husl", 256).as_hex())
  fig.patch.set_facecolor('white')
  # Use Seaborn's color palette to define colors for each unique value in the "Color" column
  color_palette = sns.color_palette("tab10", n_colors=len(df['type'].unique()))

  # Plot each color separately and add labels for the legend
  for call, color_value in zip(df['type'].unique(), color_palette):
      df_i = df[df['type'] == call]
      ax.scatter(df_i['PC1'], df_i['PC2'], df_i['PC3'], label=call, color= color_value)

  # Axis labels
  ax.set_xlabel("PC1", fontsize=14)
  ax.set_ylabel("PC2", fontsize=14)
  ax.set_zlabel("PC3", fontsize=14)

  # Plot title
  ax.set_title('PCA of bird sounds')

  # Plot legend
  plt.legend(loc="upper right")
  plt.show()

  plt.savefig("PCA_3D_plot.jpg", format="jpg", dpi=300)

def TSNEplot(embeddings_after_PCA):
  "Takes the X_PCA variable to reduce noise"
   
  tsne = TSNE(3, verbose=1)
  tsne_proj = tsne.fit_transform(embeddings_after_PCA)

  # add tSNE coordinates to our dataframe with metadata
  df = pd.DataFrame()
  df['tsne_1'] = tsne_proj[:, 0]
  df['tsne_2'] = tsne_proj[:, 1]
  df['tsne_3'] = tsne_proj[:, 2]

  fig = plt.figure(figsize=(10,10))
  ax = fig.add_subplot(111, projection='3d')
  # Pallette
  # cmap = ListedColormap(sns.color_palette("husl", 256).as_hex())
  fig.patch.set_facecolor('white')
  # Use Seaborn's color palette to define colors for each unique value in the "Color" column
  color_palette = sns.color_palette("tab10", n_colors=len(df['type'].unique()))

  # Plot each color separately and add labels for the legend
  for call, color_value in zip(df['type'].unique(), color_palette):
      df_i = df[df['type'] == call]
      ax.scatter(df_i['tsne_1'], df_i['tsne_2'], df_i['tsne_3'], label=call, color= color_value)

  # Axis labels
  ax.set_xlabel("tSNE 1", fontsize=14)
  ax.set_ylabel("tSNE 2", fontsize=14)
  ax.set_zlabel("tSNE 3", fontsize=14)

  # Plot title
  ax.set_title('tSNE of bird sounds')

  # Plot legend
  plt.legend(loc="upper right")
  plt.show()

  plt.savefig("TSNE_3D_plot.jpg", format="jpg", dpi=300)

def UMAPplot(embeddings_after_PCA):
  " Same as t-SNE "

  # Initialize UMAP with desired parameters
  umap_model = umap.UMAP(n_neighbors=15, n_components=3, metric='euclidean')

  # Compute UMAP embeddings starting from PCA-transformed data
  umap_embeddings = umap_model.fit_transform(embeddings_after_PCA)

  # add tSNE coordinates to our dataframe with metadata
  df = pd.DataFrame()
  df['umap_1'] = umap_embeddings[:, 0]
  df['umap_2'] = umap_embeddings[:, 1]
  df['umap_3'] = umap_embeddings[:, 2]

  # Create a UMAP plot
  fig = plt.figure(figsize=(10,10))
  ax = fig.add_subplot(111, projection='3d')
  # Pallette
  # cmap = ListedColormap(sns.color_palette("husl", 256).as_hex())
  fig.patch.set_facecolor('white')
  # Use Seaborn's color palette to define colors for each unique value in the "Color" column
  color_palette = sns.color_palette("tab10", n_colors=len(df['type'].unique()))

  # Plot each color separately and add labels for the legend
  for call, color_value in zip(df['type'].unique(), color_palette):
      df_i = df[df['type'] == call]
      ax.scatter(df_i['umap_1'], df_i['umap_2'], df_i['umap_3'], label=call, color= color_value)

  # Axis labels
  ax.set_xlabel("UMAP 1", fontsize=14)
  ax.set_ylabel("UMAP 2", fontsize=14)
  ax.set_zlabel("UMAP 3", fontsize=14)

  # Plot title
  ax.set_title('UMAP of bird sounds')

  # Plot legend
  plt.legend(loc="upper right")
  plt.show()

  plt.savefig("UMAP_3D_plot.jpg", format="jpg", dpi=300)


if __name__ == "__main__":
   
  # load your processed data
  barcelona_dataset = pd.read_csv("./data/euro_cities_recordings/final_data.csv")
  metadata_columns = barcelona_dataset.columns
  embeddings = run_model(barcelona_dataset, metadata_columns)
  embeddings.to_csv("./data/euro_cities_recordings/LS_embeddings.csv", header=True, index=False)
  print(embeddings.head(10))
