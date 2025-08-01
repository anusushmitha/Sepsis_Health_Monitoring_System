import numpy as np

import pandas as pd

from sklearn.ensemble import RandomForestClassifier

from sklearn.model_selection import train_test_split

import json



# Generate synthetic data

np.random.seed(42)

num_samples = 1000



# Features: temperature, bpm, spo2

temp = np.random.uniform(25, 40, num_samples)  # temperature between 25 and 40°C

bpm = np.random.randint(50, 120, num_samples)  # heart rate between 50 and 120 bpm

spo2 = np.random.uniform(85, 100, num_samples) # oxygen saturation between 85 and 100%



# Label: 1 if sepsis, 0 otherwise

labels = [

    1 if (t < 30 or t > 35) and (b < 70 or b > 100) and s < 90 else 0

    for t, b, s in zip(temp, bpm, spo2)

]



# Create a DataFrame

data = pd.DataFrame({'temp': temp, 'bpm': bpm, 'spo2': spo2, 'sepsis': labels})



# Split the data into features and target

X = data[['temp', 'bpm', 'spo2']]

y = data['sepsis']



# Train a random forest classifier

model = RandomForestClassifier(n_estimators=100, random_state=42)

model.fit(X, y)



# Save the trained model

trees = []

for tree in model.estimators_:

    tree_dict = {

        'children_left': tree.tree_.children_left.tolist(),

        'children_right': tree.tree_.children_right.tolist(),

        'feature': tree.tree_.feature.tolist(),

        'threshold': tree.tree_.threshold.tolist(),

        'value': tree.tree_.value.tolist(),

    }

    trees.append(tree_dict)



# Save the model as a JSON file

with open('random_forest_sepsis.json', 'w') as f:

    json.dump({'trees': trees}, f)



print("Model training complete and saved as random_forest_sepsis.json")
