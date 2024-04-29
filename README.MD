# GTKWave-Plus

GTKWave-Plus allows users to detect circular and linear dependencies while visualizing waveforms, similar to GTKWave. It provides a user-friendly interface for viewing signals and their dependencies, zooming in and out on the timeline, and manipulating signal data. Users can detect circular dependencies as well as analyzing how signals affect other signals.

## Features
- **Signal Visualization**: View waveforms of signals loaded from a CSV file.
- **Signal Expansion**: Double-click on signals to expand multibit signals into individual bits.
- **Signal Dependencies**: View and explore dependencies of signals. This includes linear and circular dependencies
- **Zoom Control**: Navigate through the timeline by moving forward or backward.
- **Signal Manipulation**: Add, delete, and save signals.

## Getting Started

1. **Prerequisites**: Make sure you have Java and Python installed on your system.
   
2. **Clone the Repository**: Clone this repository to your local machine using the following command:
    ```
    git clone <repository_url>
    ```
3. **Install Required Python Libraries:**
    ```
    pip install -r requirements.txt
    ```

4. **Add jar libraries in lib folder to project classpath**: Go to Referenced Libraries, under Java Projects in the Explorer pane and click the "+" button to add add jar libraries to the project classpath. Select all of the files inside the lib folder.

5. **Process VCD File:** Process the VCD file using the following command
    ``` 
    python3 process_vcd.py <vcd_file>
    ```

6. **Run**: Run the launch configuration in the .vscode folder!


## Demo
[![demo](https://img.youtube.com/vi/7KfJ9n2TlhA/0.jpg)](https://www.youtube.com/watch?v=7KfJ9n2TlhA)
