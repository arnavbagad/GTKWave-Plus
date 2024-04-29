# GTKWave

GTKWave is a JavaFX application designed to visualize waveforms represented in a CSV file format. It provides a user-friendly interface for viewing signals and their dependencies, zooming in and out on the timeline, and manipulating signal data. Users can detect circular dependencies as well as analyzing how signals affect other signals.

## Features

- **Signal Visualization**: View waveforms of signals loaded from a CSV file.
- **Signal Expansion**: Double-click on signals to expand multibit signals into individual bits.
- **Signal Dependencies**: View and explore dependencies of signals.
- **Zoom Control**: Navigate through the timeline by moving forward or backward.
- **Signal Manipulation**: Add, delete, and save signals.

## Getting Started

1. **Prerequisites**: Make sure you have Java installed on your system.
   
2. **Clone the Repository**: Clone this repository to your local machine using the following command:
    ```
    git clone <repository_url>
    ```

3. **Set your launch.json**: Create a launch.json file under the ".vscode" folder and add the following run configuration:
    ```
    {
            "type": "java",
            "name": "Launch App",
            "request": "launch",
            "mainClass": "${file}",
            "vmArgs": "--module-path ${workspaceFolder}/lib --add-modules javafx.controls,javafx.fxml"
    }   
    ```

4. **Add lib folder to referenced libraries**: Go to Referenced Libraries, under Java Projects in the Explorer pane and click the "+" button to add add jar libraries to the project classpath. Select all of the files inside the lib folder.

5. **Run**: Run the launch configuration from above!

## Usage

- **Signal ListView**: Displays a list of loaded signals. Double-click on a signal to expand it into individual bits.
  
- **WavePane**: Visualizes the waveforms of the loaded signals. Click on the timeline to view data values at specific time points.

- **Menu Bar**: Provides options for file operations (new, open, save, exit) and help (about).

## Contributing

Contributions are welcome! If you have any ideas for improvements, bug fixes, or new features, feel free to submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
