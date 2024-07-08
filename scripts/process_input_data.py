import os, json
import pandas as pd

# Example usage:
metadata_path = './data/euro_cities_recordings/euro_cities.csv'
folder_path = './data/euro_cities_recordings'
meta_data = pd.read_table(metadata_path, delimiter=',', header=0)



# first create a dictionary with the mp3 IDs and their paths

def create_mp3_dict(root_folder):
    mp3_dict = {}
    
    # Walk through all sub-folders and files
    for subdir, _, files in os.walk(root_folder):
        for file in files:
            if file.endswith('.mp3'):
                # Extract the ID from the filename (assuming filename format is "ID.mp3")
                file_id = os.path.splitext(file)[0]
                # Get the absolute path to the MP3 file
                file_path = os.path.relpath(os.path.join(subdir, file)) # relative path
                # file_path = os.path.abspath(os.path.join(subdir, file)) # absolute path
                # Add to the dictionary
                mp3_dict[file_id] = file_path
    
    return mp3_dict

def save_dict_to_json(mp3_dict, json_file_path):
    # Save the dictionary to a JSON file
    with open(json_file_path, 'w') as json_file:
        json.dump(mp3_dict, json_file)

def load_dict_from_json(json_file_path):
    # Load the dictionary from a JSON file
    with open(json_file_path, 'r') as json_file:
        mp3_dict = json.load(json_file)
    return mp3_dict

def add_mp3_path_column(df, mp3_dict):
    # Add a new column to the DataFrame based on the dictionary
    df['filepath'] = df['id'].apply(lambda x: mp3_dict.get(str(x), None))
    # df['filepath'] = df['filepath'].astype(str)
    return df

# Example usage
json_file_path = './dataset_barcelona_2/mp3dict.json'
mp3_files_dict = create_mp3_dict(folder_path)
save_dict_to_json(mp3_files_dict, json_file_path)
loaded_mp3_files_dict = load_dict_from_json(json_file_path)

final_df = add_mp3_path_column(meta_data, loaded_mp3_files_dict)

# add filename column
final_df["filename"] = final_df["id"].astype(str) + ".mp3"
final_df['filename'] = final_df['filename'].astype(str)
final_df = final_df.dropna(subset=["filepath"])


print(final_df.head(10))

final_df.to_csv("./data/euro_cities_recordings/final_data.csv", header=True, index=False)