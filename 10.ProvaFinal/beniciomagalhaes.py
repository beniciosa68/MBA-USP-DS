#!/usr/bin/env python
# coding: utf-8

# In[1]:


#Benicio Ramos Magalhaes


# #### Questão 2.1 - O arquivo contratacaocorona-27-07-acertado.csv contém informações de compras emergenciais ligadas a COVID 19.
# 
# 1. Leia o arquivo e considere apenas as colunas 'QUANTIDADE', 'VALOR_UNITM-ARIO' e 'VALOR_TOTAL'. Verifique quantos dados faltantes existem no DataFrame resultante.
# 
# 2. Remova as linhas do data frame que contenham dados faltantes. Verifique quantas linhas foram removidas.
# 
# 3. Prepare a coluna 'VALOR_TOTAL' para ser processada como numérica, e a seguir busque por outliers presentes nesta coluna. Para isso use o método do desvio padrão com σ = 3 . Verifique o número de outliers encontrados.
# 
# Com base nos itens acima, assinale a alternativa correta:
# 
# Escolha uma opção:<br>
# a. Dados faltantes: 12, Linhas removidas: 12, Outliers encontrados: 6<br>
# b. Dados faltantes: 12, Linhas removidas: 12, Outliers encontrados: 4<br>
# c. Dados faltantes: 30, Linhas removidas: 24, Outliers encontrados: 6<br>
# <b>d. Dados faltantes: 30, Linhas removidas: 12, Outliers encontrados: 4<br></b>
# e. Dados faltantes: 30, Linhas removidas: 30, Outliers encontrados: 2<br>

# In[2]:


import pandas as pd


# In[3]:


#1. Leia o arquivo e considere apenas as colunas 'QUANTIDADE', 'VALOR_UNITM-ARIO' e 'VALOR_TOTAL'. 
#Verifique quantos dados faltantes existem no DataFrame resultante.

df = pd.read_csv('contratacaocorona-27-07-acertado.csv', sep=';')
data = df[['QUANTIDADE','VALOR_UNITM-ARIO','VALOR_TOTAL']]

numLinhas = data.shape[0]
print('Número de linhas atuais:',data.shape[0])
print()
#verificando quantidade de dados faltantes
print('Qtd de dados faltantes:')
print(data.isnull().sum())
print()
print('Total:',12+12+6)


# In[4]:


#2. Remova as linhas do data frame que contenham dados faltantes. Verifique quantas linhas foram removidas.

# remove colunas que contenham algum dado faltante
data = data.dropna()
numLinhasAtual = data.shape[0]

print('Total de linhas removidas:', numLinhas - numLinhasAtual)


# In[5]:


#3. Prepare a coluna 'VALOR_TOTAL' para ser processada como numérica, e a seguir busque por outliers presentes nesta coluna. 
#Para isso use o método do desvio padrão com σ = 3 . Verifique o número de outliers encontrados.

data['VALOR_TOTAL'] = data['VALOR_TOTAL'].str.replace(',', '')
data['VALOR_TOTAL'] = data['VALOR_TOTAL'].str.replace(' ', '')
data['VALOR_TOTAL'] = data['VALOR_TOTAL'].str.replace('R', '')
data['VALOR_TOTAL'] = data['VALOR_TOTAL'].str.replace('$', '')
data['VALOR_TOTAL'] = data['VALOR_TOTAL'].str.replace('.', '')
data['VALOR_TOTAL'] = pd.to_numeric(data['VALOR_TOTAL'])

#encontrando os outliers da coluna
desvp = data['VALOR_TOTAL'].std()
media = data['VALOR_TOTAL'].mean()
sigma = 3

outliers_std = data[(data['VALOR_TOTAL'] < media-(desvp*sigma)) 
                   | (data['VALOR_TOTAL'] > media+(desvp*sigma))]

print('Quantidade de outliers:',len(outliers_std))


# In[ ]:




