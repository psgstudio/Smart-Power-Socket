import requests
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import tensorflow as tf
from tensorflow import keras
from sklearn.preprocessing import MinMaxScaler
from sklearn.model_selection import train_test_split

# Define your ThingSpeak channel ID and API key
channel_id = '1713410'
api_key = 'W3KYUV68TXQ76IQ8'

# Define the ThingSpeak URL for your channel
url = f'https://api.thingspeak.com/channels/{channel_id}/feeds.json?api_key={api_key}'

# Make a GET request to fetch the data
response = requests.get(url)

# Extract the data in JSON format
data = response.json()

# Create a DataFrame from the data
df = pd.DataFrame(data['feeds'])

# Extract relevant fields (e.g., 'field3' for power consumption and 'created_at' for timestamps)
df['created_at'] = pd.to_datetime(df['created_at'])
df = df[['field3', 'created_at']].dropna()

# Sort the DataFrame by timestamp
df.sort_values(by='created_at', inplace=True)
df.set_index('created_at', inplace=True)

# Normalize the data
scaler = MinMaxScaler()
df['field3'] = scaler.fit_transform(df['field3'].values.reshape(-1, 1))

# Create sequences for LSTM training
sequence_length = 10  # Adjust as needed
X, y = [], []

for i in range(len(df) - sequence_length):
    X.append(df['field3'].iloc[i:i+sequence_length].values)
    y.append(df['field3'].iloc[i+sequence_length])

X = np.array(X)
y = np.array(y)

# Split the data into training and validation sets
X_train, X_val, y_train, y_val = train_test_split(X, y, test_size=0.2, random_state=42)

# Define the LSTM model
model = keras.Sequential([
    keras.layers.LSTM(64, return_sequences=True, input_shape=(sequence_length, 1)),
    keras.layers.LSTM(32),
    keras.layers.Dense(1)
])

model.compile(optimizer='adam', loss='mean_squared_error')

# Train the model
history = model.fit(X_train, y_train, epochs=50, batch_size=64, validation_data=(X_val, y_val))

# Make predictions
y_pred = model.predict(X_val)

# Inverse transform the scaled data to get actual values
y_pred = scaler.inverse_transform(y_pred)
y_val = scaler.inverse_transform(y_val.reshape(-1, 1))

# Plot the results
plt.figure(figsize=(12, 6))
plt.plot(df.index[-len(y_val):], y_val, label='Actual Power Consumption', color='blue')
plt.plot(df.index[-len(y_pred):], y_pred, label='Predicted Power Consumption', color='red')
plt.xlabel('Timestamp')
plt.ylabel('Power Consumption')
plt.legend()
plt.title('Actual vs. Predicted Power Consumption')
plt.grid(True)

# Save the plot
plt.savefig('img.png')

plt.show()