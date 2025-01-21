# -*- coding: utf-8 -*-
"""
Created on Wed Nov 20 18:16:04 2024

@author: utente
"""

import pandas as pd
import matplotlib.pyplot as plt

# Caricare il file CSV con il nome di output dello script
file_path = './result3.csv'
df = pd.read_csv(file_path)

# Convertire la colonna 'travel_time' in formato timedelta e poi in secondi
df['travel_time_seconds'] = pd.to_timedelta(df['travel_time']).dt.total_seconds()

# Creare l'istogramma dei tempi di viaggio
plt.figure(figsize=(10, 6))
plt.hist(df['travel_time_seconds'], bins=20, edgecolor='black')
plt.title('Histogram of Travel Time per Order')
plt.xlabel('Travel Time (seconds)')
plt.ylabel('Frequency') #quantity?
plt.grid(True)
plt.show()
