from xenopy import Query
import pandas as pd
import os

def get_xc_data(query_object, metadata_filename, recordings_dir):
    """
    automated pipeline for downloading from the XC database
    """
    q = query_object

    # retrieve metadata
    metafiles = q.retrieve_meta(verbose=True, attempts=10)
    extra_data = {}

    for files in metafiles.items():
        if files[0]=="recordings":
            metadata_df = pd.DataFrame(files[1])
        else:
            extra_data[files[0]] = files[1]
    
    extra_data_df = pd.DataFrame.from_dict(extra_data, orient="index")
    extra_data_df.to_csv("metadata_" + metadata_filename, mode="a", header=True, index=False, sep=",")
    metadata_df.to_csv(os.path.join(recordings_dir, metadata_filename), mode="a", header=True, index=False, sep=",")

    # retrieve recordings
    q.retrieve_recordings(multiprocess=True, nproc=12, attempts=10, outdir=recordings_dir)

if __name__ == "__main__":

    # a. Everything that's in Barcelona (and surroundings)
    # query_bcn = Query(box="41.208,1.836,41.566,2.508", q_gt="C") 
    # # this gets all data in a given area box, with an audio quality higher or equal to C 
    # get_xc_data(query_1city, metadata_filename="barcelona_metadata_3.csv", recordings_dir="./dataset_barcelona_2/")


    # query_1bird = Query(name="Common Eider")
    # get_xc_data(query_1bird, metadata_filename="common_eider_metadata.csv", recordings_dir="./data/common_eider_recordings/")

    # query_1genus = Query(gen="Tachycineta") 
    # get_xc_data(query_1genus, metadata_filename="Tachycineta_genus_metadata.csv", recordings_dir="./data/Tachycineta_genus_recordings/")


    # A couple of European cities

    query_BCN = Query(box="1.111,1.408,41.821,2.658") # q_gt="C" BARCELONA
    query_VLC = Query(box="39.369,-0.474,39.54,-0.311") # q_gt="C" VALENCIA
    query_MAD = Query(box="40.084,-4.265,41.027,-3.172") # q_gt="C" MADRID
    query_PRS = Query(box="48.602,1.978,49.03,2.667") # q_gt="C" PARIS
    query_LDN = Query(box="51.302,-0.582,51.672,0.272") # q_gt="C" LONDON
    query_KPR = Query(box="45.504,13.667,45.567,13.797") # q_gt="C" KOPER
    query_OSL = Query(box="59.786,10.574,59.965,10.933") # q_gt="C" OSLO
    query_BLN = Query(box="52.374,13.152,52.689,13.701") # q_gt="C" BERLIN
    query_ROM = Query(box="41.591,12.198,42.05,13.077") # q_gt="C" ROME

    euro_cities_query = [query_BCN, query_VLC, query_MAD, query_PRS, query_LDN, query_KPR, query_OSL, query_BLN, query_ROM]

    for ix, city in enumerate(euro_cities_query):
        get_xc_data(city, metadata_filename=f"euro_cities.csv", recordings_dir="./data/euro_cities_recordings/")
    

    query_MEDITERRANEAN = Query(box="29.18,-1.261,42.899,39.696") # q_gt="C" MEDITERRANEAN SEA
    get_xc_data(city, metadata_filename=f"mediterranean_sea.csv", recordings_dir="./data/mediterranean_sea/")
