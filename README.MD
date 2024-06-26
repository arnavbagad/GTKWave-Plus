# GTKWave-Plus

GTKWave-Plus allows users to detect circular and linear dependencies while visualizing waveforms, similar to GTKWave. It provides a user-friendly interface for viewing signals and their dependencies, zooming in and out on the timeline, and manipulating signal data. Users can detect circular dependencies as well as analyzing how signals affect other signals.

## Features
- **Signal Visualization**: View waveforms of signals loaded from a CSV file.
- **Signal Expansion**: Double-click on signals to expand multibit signals into individual bits.
- **Signal Dependencies**: View and explore dependencies of signals. This includes linear and circular dependencies
- **Zoom Control**: Navigate through the timeline by moving forward or backward.
- **Signal Manipulation**: Add, delete, and save signals.

## Getting Started

1. **Prerequisites**: Make sure you have Java and Python installed on your system. The MakeFile currently only runs on the lab machines and the Python Libraries require Python 3.11.
   
2. **Clone the Repository**: Clone this repository to your local machine using the following command:
    ```
    git clone https://github.com/arnavbagad/GTKWave-Plus
    ```
3. **Install Required Python Libraries:**
    ```
    pip install -r requirements.txt
    ```

4. **Add jar libraries in lib folder to project classpath**: Go to Referenced Libraries, under Java Projects in the Explorer pane and click the "+" button to add add jar libraries to the project classpath. Select all of the files inside the lib folder.

5. **Adding .vcd file**: Add your .vcd file to the directory.

6. **Adding main module**: Add the main module code to a file named test.main

6. **Dependency Tree**: make test creates the dependency tree and outputs circular dependencies to the test.txt file.

7. **Process VCD File:** Process the VCD file using the following command
    ``` 
    python3 process_vcd.py <vcd_file>
    ```
8. **Run**: Run the launch configuration in the .vscode folder!


## Demo

Click to play the video.
[![demo](https://img.youtube.com/vi/7KfJ9n2TlhA/0.jpg)](https://www.youtube.com/watch?v=7KfJ9n2TlhA)
