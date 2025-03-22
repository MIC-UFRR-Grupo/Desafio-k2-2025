from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

dados_ecg = []  # Lista para armazenar os dados do ECG e o ID do usuário

@app.route('/dados', methods=['POST'])
def receber_dados():
    global dados_ecg
    json_data = request.get_json()
    
    # Verifica se os dados contêm ECG e ID
    if "ecg" in json_data and "id" in json_data:
        dados_ecg.append({"ecg": json_data["ecg"], "id": json_data["id"]})
        
        # Mantém os últimos 100 valores
        if len(dados_ecg) > 100:
            dados_ecg.pop(0)
    
    return jsonify({"status": "ok"})

@app.route('/dados', methods=['GET'])
def enviar_dados():
    return jsonify(dados_ecg)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
