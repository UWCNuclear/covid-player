# Create your views here.
import requests
import re
import os
import numpy as np
import glob
import pandas as pd
import time

from bs4 import BeautifulSoup
from tqdm import tqdm
from time import gmtime, strftime

# =============================================================================

baseurl = "https://www.worldometers.info/coronavirus/"

# check that the environment has been correclty defined
try:
    os.environ['COVID_PLAYER_SYS']
except Exception as e:
    print("ERROR - You need to load the covid-player environment...")
    exit()

datapath = os.path.join(os.environ['COVID_PLAYER_SYS'], "database")

# check that the folder containing the databases exists
if not os.path.exists(datapath):
    print(datapath + " not existing, something strange happens")
    exit()

# worldometers url
baseurl = "https://www.worldometers.info/coronavirus/"

start = time.time()

# list of links for each country
df_country_list = pd.read_csv(os.path.join(datapath, 'country_list.csv'))[['country','link']]

selected_countries = "USA ; Brazil ; India ; Mexico ; UK ; Italy ; Spain ; France ; Peru ; Iran ; Colombia ; Argentina ; Russia ; South Africa ; Chile ; Indonesia ; Ecuador ; Belgium ; Iraq	; Germany ; Canada ; Turkey ; Bolivia ; Netherlands ; Philippines ; Pakistan ; Egypt ; Romania ; Ukraine ; Sweden ; Bangladesh ; Saudi Arabia ; China ; Poland ; Guatemala ; Morocco ; Panama ; Honduras ; Israel ; Portugal ; Dominican Republic ; Switzerland"

def main():
    for i, row in df_country_list.iterrows():
        c_name = row['country']
        link = row['link']
        if(selected_countries.find(c_name) >= 0):
            print(i, c_name, end=" ")
        else:
            continue

        df = pd.DataFrame()

        link = baseurl+link
        try:
            html_page = load_data(link)
            res = html_parser(html_page.content, 'script')
        except Exception as e:
            country_lst_issue.append([i,c_name,link])
            print(" :( :( :( ")
            continue

#    date_range, data_daily_cases = clean_data(res, 'Daily New Cases')
#    if date_range != False:
#        df['Date'] = date_range
#        df['Daily Cases'] = data_daily_cases
#
        date_range, data_total_cases = clean_data(res, 'Total Cases')
        if date_range != False:
            df['Date'] = date_range
            df['Total Cases'] = data_total_cases

#    date_range, data_daily_deaths = clean_data(res, 'Daily Deaths')
#    if date_range != False:
#        df['Daily Deaths'] = data_daily_deaths
#
        date_range, data_total_deaths = clean_data(res, 'Total Deaths')
        if date_range != False:
            df['Total Deaths'] = data_total_deaths
#
#    date_range, data_active = clean_data(res, 'Active Cases')
#    if date_range != False:
#        df['Active Cases'] = data_active

#    if not(date_range and data_daily_cases and data_daily_deaths and data_active):
#        country_lst_issue.append([i,c_name,link])
#        print(" :( :( :( ",end="")

        c_name = c_name.replace(" ","_")
        output_file = datapath + "/" + c_name + '.csv'
        df.to_csv(output_file)
        print(output_file)


def load_data(link):
    
    try:
        html_page = requests.get(link)
    except requests.exceptions.RequestException as e:
        print (e)

    return html_page


def html_parser(content, tag, class_=False, id_=False):
    
    bs = BeautifulSoup(content, 'html.parser')
    
    if class_:
        search = bs.find_all(tag, class_=class_)
    elif id_:
        search = bs.find_all(id=id_)
    else:
        search = bs.select(tag)
    
    return search


def clean_data(res, info_label):
    
    # find the right sciprt tag
    data = False
    for i, r in enumerate(res):
        r = str(r)
        if r.find(info_label) != -1:
            data = r.split("\n")
            # print(i)
            break
    
    if data == False:
        return False, False
    
    # find the right line inside script tag
    for i,d in enumerate(data):
        if d.find('xAxis') != -1:
            date_range = data[i+1]
            date_range = re.search(r"(?<=\[).*?(?=\])", date_range).group(0).split(",")
            date_range = [t.strip('\"') for t in date_range]
            # print(i)
        elif d.find('series') != -1:
            data_range = data[i+4]
            data_range = re.search(r"(?<=\[).*?(?=\])", data_range).group(0).split(",")
            data_range = [int(t) if t != 'null' else 0 for t in data_range]
            # print(i)
    
    return date_range, data_range


main()

# =============================================================================
# get list of countries and links
# run get_country_list() and then comment it.
# then run 2nd step
# =============================================================================

# =============================================================================
# def get_country_list():
#     
#     link = "https://www.worldometers.info/coronavirus/"
#     
#     html_page = load_data(link)
#     
#     res = html_parser(html_page.content, 'a', class_='mt_a')
#     
#     country_lst = {}
#     
#     for r in tqdm(res):
#         country_lst.update({r.get_text():r.get('href')}) 
#     
#     df_country = pd.DataFrame()
#     
#     for k,v in country_lst.items():
#         df_country = df_country.append([[k,v]], ignore_index='True')
#     df_country.columns = ['country','link']
#     
#     return df_country
# 
# df_country_list = get_country_list()
# df_country_list.to_csv(os.path.join(datapath, 'country_list.csv'))

print("\n\nTime taken: {} seconds".format(round(time.time()-start, 2)))
