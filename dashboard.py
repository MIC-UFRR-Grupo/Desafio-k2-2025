import streamlit as st
import requests
import pandas as pd
import plotly.express as px
import time

API_URL = "http://172.22.68.45:5000/dados"  # URL do servidor Flask

st.title("Monitor de ECG - AD8232 com ESP32")

# Função para buscar os dados do servidor com cache
@st.cache_data(ttl=1)  # Atualiza os dados a cada 1 segundo
def get_ecg_data():
    response = requests.get(API_URL)
    if response.status_code == 200:
        return response.json()
    return []

# Criar o espaço para o gráfico
chart_placeholder = st.empty()

while True:
    ecg_data = get_ecg_data()

    if ecg_data:
        # Cria um DataFrame com os dados do ECG e o ID do usuário
        df = pd.DataFrame(ecg_data)
        
        # Exibe o ID do usuário atual
        if len(df) > 0:
            current_user_id = df.iloc[-1]["id"]  # Pega o ID do último dado recebido
            st.sidebar.markdown(f"**Usuário Autenticado:** ID {current_user_id}")
        
        # Cria o gráfico de ECG
        fig = px.line(df, y="ecg", title="Sinal de ECG em Tempo Real")
        unique_key = f"ecg_chart_{int(time.time())}"
        chart_placeholder.plotly_chart(fig, use_container_width=True, key=unique_key)
    
    time.sleep(1)  # Atualiza a cada 1 segundo
