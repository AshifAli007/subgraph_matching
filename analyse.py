import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file into a pandas DataFrame
df = pd.read_csv('output.csv')

# Get the number of rows in the DataFrame
num_rows = len(df)

# Create a multiline graph with a line for each row
for i in range(100):
    row = df.iloc[i]
    labels = ['Matches', 'Recursive calls', 'Total time', 'Search time']
    values = [row['Matches'], row['Recursive calls'], row['Total time'], row['Search time']]

    plt.plot(labels, values, label=f'Row {i + 1}')

# Add legend, title, and labels
plt.legend()
plt.title('Metrics for Each Row')
plt.xlabel('Metrics')
plt.ylabel('Values')

# Display the multiline graph
plt.show()
