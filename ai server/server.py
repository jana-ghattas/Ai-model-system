from flask import Flask, request
import tensorflow as tf
import numpy as np
import cv2
import tf_keras as keras

app = Flask(__name__)

@app.route('/')
def home():
    return "<h1>AI Greenhouse Server is Running!</h1><p>Waiting for images from ESP32-CAM...</p>"

# Use legacy loader to fix 'DepthwiseConv2D' error
model = keras.models.load_model("keras_model.h5", compile=False)

@app.route('/upload', methods=['POST'])
def upload():
    try:
        file_bytes = request.data
        if not file_bytes:
            return "NO_DATA"

        npimg = np.frombuffer(file_bytes, np.uint8)
        img = cv2.imdecode(npimg, cv2.IMREAD_COLOR)
        
        if img is None:
            return "ERROR_DECODE"

        img = cv2.resize(img, (224,224))
        img = img / 255.0
        img = np.expand_dims(img, axis=0)

        prediction = model.predict(img)

        classes = ["DRY", "HEALTHY", "STRESSED"]
        result = classes[np.argmax(prediction)]

        print(f"AI Result: {result}")
        return result
    except Exception as e:
        print(f"Server Error: {e}")
        return "SERVER_ERROR"

app.run(host="0.0.0.0", port=5000)
'''from flask import Flask, request
import numpy as np
import cv2

app = Flask(__name__)

def analyze_image(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    avg = np.mean(gray)

    if avg < 70:
        return "DRY"
    elif avg < 140:
        return "STRESSED"
    else:
        return "HEALTHY"


@app.route('/')
def home():
    return "SERVER RUNNING"


@app.route('/upload', methods=['POST'])
def upload():
    try:
        data = request.data

        npimg = np.frombuffer(data, np.uint8)
        img = cv2.imdecode(npimg, cv2.IMREAD_COLOR)

        if img is None:
            return "ERROR"

        result = analyze_image(img)

        print("RESULT:", result)
        return result

    except:
        return "SERVER ERROR"


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000) (from chat without tf)'''