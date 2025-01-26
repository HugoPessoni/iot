import pandas as pd
import re

# Função para converter o nome das colunas
def clean_column_name(column_name):
    # Remove espaços em branco e caracteres especiais
    column_name = re.sub(r'\W+', '_', column_name)
    # Converte para minúsculas
    column_name = column_name.lower()
    return column_name

# Carregar o arquivo CSV
df = pd.read_csv('material_imol_original.csv')

# Renomear as colunas usando a função
df.columns = [clean_column_name(col) for col in df.columns]

# Exibir as primeiras linhas do DataFrame para verificar
print(df.head())

# Salvar o DataFrame em um novo arquivo CSV
df.to_csv('material_imol_ia.csv', index=False)