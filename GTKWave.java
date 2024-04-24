import javafx.application.*;
import javafx.stage.*;
import javafx.collections.*;

import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.input.*;
import javafx.scene.paint.*;
import javafx.scene.shape.*;
import javafx.scene.text.*;

import java.io.*;
import java.util.*;


public class GTKWave extends Application {

    private ArrayList<Signal> signalList;
    private ListView<String> signalListView;
    private FlowPane wavePane;
    private int start = 0;
    private int cellHeight = 30;
    private int WIDTH_NUMBER = 12;
    private int MENU_WIDTH = 200;
    private int TIME_WIDTH = 0;
    private Line currentVerticalLine = null;
    private int TIME_STEP = 500;
    private HashMap<String, String[]> ALL_SIGNALS = new HashMap<>();
    private BorderPane rootLayout = null;

    @Override
    public void start(Stage primaryStage) {
        primaryStage.setMaximized(true);
        // Initialize the data structure with sample data
        initializeData();

        // SIGNALLISTVIEW
        signalListView = new ListView<>();
        signalListView.getItems().addAll(Signal.names);
        signalListView.setPrefWidth(MENU_WIDTH);
        signalListView.getItems().add(0, "SIGNAL");

        // EXPAND MULTIBIT
        signalListView.setOnMouseClicked(event -> {
            if (event.getButton().equals(MouseButton.PRIMARY) && event.getClickCount() == 2) {
                // Double click detected
                int selectedIndex = signalListView.getSelectionModel().getSelectedIndex();

                if (selectedIndex > 0) {
                    String selectedItem = signalListView.getItems().get(selectedIndex);
                        // You can change the item here, for example:
                        // split the signal into multiple signals
                    Signal multidata = Signal.signalMap.get(selectedItem);
                    if (multidata.singleBit) return;

                    if (selectedIndex == signalListView.getItems().size() - 1 || !Signal.signalMap.get(signalListView.getItems().get(selectedIndex + 1)).fake) {
                        System.out.println("Double click on: " + selectedItem);
                        ArrayList<String> newSignalNames = new ArrayList<>();
                        ArrayList<Signal> newSignals = new ArrayList<>();

                        for (int i = 0; i < findDataTimeLength(multidata.data); i++) {
                            String name = "\t" + selectedItem + "[" + i + "]";
                            newSignalNames.add(name);
                            Signal signal = new Signal(name, arraySplicer(multidata.data, i));
                            signal.fake = true;
                            newSignals.add(signal);
                        }
                        signalListView.getItems().addAll(selectedIndex + 1, newSignalNames);
                        signalList.addAll(selectedIndex, newSignals);
                        generateWaves();                    
                        // insert signal into the signal list
                        // change signal display
                    }
                    else {
                        // delete from signalMap
                        // delete from signalList
                        // delete from signalListView
                        for (int i = 0; i < findDataTimeLength(multidata.data); i++) {
                            String name = "\t" + selectedItem + "[" + i + "]";
                            signalList.remove(Signal.signalMap.get(name));
                            Signal.signalMap.remove(name);
                            signalListView.getItems().remove(name);
                        }
                        generateWaves();
                    }
                }
            }
            signalListView.setPrefWidth(MENU_WIDTH);
        });

        signalListView.setCellFactory(listView -> {
            ListCell<String> cell = new ListCell<String>() {
                @Override
                protected void updateItem(String item, boolean empty) {
                    super.updateItem(item, empty);
                    if (empty || item == null) {
                        setText(null);
                        setGraphic(null);
                    } else {
                        setText(item);
                        if (getIndex() == 0) {
                            setStyle("-fx-font-weight: bold; -fx-text-fill: darkblue; -fx-alignment: center; -fx-font-size: 14;");
                        } else {
                            setStyle(null); // Reset style for other items
                        }
                    }
                }
            };
            // Set preferred height for each cell
            cell.setPrefHeight(cellHeight); // Example height of 30 pixels
            return cell;
        });

        ContextMenu contextMenu = new ContextMenu();
        MenuItem deleteItem = new MenuItem("Delete");
        deleteItem.setOnAction(e -> {
            String selectedItem = signalListView.getSelectionModel().getSelectedItem();
            if (selectedItem != null) {
                signalListView.getItems().remove(selectedItem);
                signalList.remove(Signal.signalMap.get(selectedItem));
                Signal.signalMap.remove(selectedItem);
                generateWaves();
            }
        });
        contextMenu.getItems().add(deleteItem);

        signalListView.setOnContextMenuRequested(e -> {
            contextMenu.show(signalListView, e.getScreenX(), e.getScreenY());
        });
        signalListView.setCellFactory(param -> new DragAndDropListCell());

        // WAVE PANE
        wavePane = new FlowPane();
        wavePane.setStyle("-fx-background-color: lightgrey;");
        HBox.setHgrow(wavePane, Priority.ALWAYS); 
        generateWaves();

        // Create the root layout (HBox) and add the sections
        // HBox mainLayout = new HBox(signalListView, stackPane);
        HBox mainLayout = new HBox(signalListView, wavePane);

        // TOP PANE
        HBox topPane = new HBox(10); // Add 10 pixels spacing between children
        topPane.setStyle("-fx-padding: 5; -fx-alignment: center-left;");
    
        // MENU BAR
        MenuBar menuBar = createMenuBar(primaryStage);
        menuBar.setPrefWidth(MENU_WIDTH - 10); // Set preferred width of the menu bar to 200
    
        // FORWARD
        Button forwardButton = new Button("Forward");
        forwardButton.setOnAction(e -> {
            // Define the action for the FORWARD button here
            // System.out.println("FORWARD button clicked");
            if (start + WIDTH_NUMBER >= TIME_WIDTH) return;
            
            start += WIDTH_NUMBER/2;
            generateWaves();
            rootLayout.getChildren().remove(currentVerticalLine);

        });

        Button backwardButton = new Button("Backward");
        backwardButton.setOnAction(e -> {
            // Define the action for the FORWARD button here
            // System.out.println("FORWARD button clicked");
            if (start - WIDTH_NUMBER/2 < 0) return;
            start -= WIDTH_NUMBER/2;
            generateWaves();    
            rootLayout.getChildren().remove(currentVerticalLine);
    
        });
    
        TextField textField = new TextField();
        textField.setOnAction(e -> {
            String inputText = textField.getText();
            System.out.println("Entered: " + inputText);
            addSignal(inputText);
            textField.clear();
        });

        // Add the menu bar, FORWARD button, BACKWARD button, and text field to the top pane
        topPane.getChildren().addAll(menuBar, backwardButton, forwardButton, textField);
    

        // Create the root layout (BorderPane) and set the top as the MenuBar
        rootLayout = new BorderPane();
        rootLayout.setTop(topPane);
        // rootLayout.setTop(createMenuBar(primaryStage));
        rootLayout.setCenter(mainLayout);
        // VERTICAL LINE
        wavePane.setOnMouseClicked(event -> {
            // Get the x coordinate of the click event
            double x = event.getX();
            if (currentVerticalLine != null) {
                rootLayout.getChildren().remove(currentVerticalLine);
            }

            int numCell = (int) (x/ (wavePane.getWidth() / WIDTH_NUMBER));
            System.out.println(numCell);

            // Create a vertical line at the x coordinate
            Line verticalLine = new Line(x+MENU_WIDTH, 0, x+MENU_WIDTH, wavePane.getHeight());
            verticalLine.setStroke(Color.BLACK); // Set the line color (e.g., red)
            verticalLine.setStrokeWidth(0.5); // Set the line width (e.g., 1)
            
            // Add the vertical line to the wavePane
            rootLayout.getChildren().add(verticalLine);
            currentVerticalLine = verticalLine;

            
            signalListView.setCellFactory(listView -> {
                ListCell<String> cell = new ListCell<String>() {
                    @Override
                    protected void updateItem(String item, boolean empty) {
                        super.updateItem(item, empty);
                        if (empty || item == null) {
                            setText(null);
                            setGraphic(null);
                        } else {
                            setText(item);
                            if (getIndex() == 0) {
                                setStyle("-fx-font-weight: bold; -fx-text-fill: darkblue; -fx-alignment: center; -fx-font-size: 14;");
                            } else {
                                String dataVal;
                                int binDataVal = 0;
                                if (Signal.signalMap.get(item).data[numCell+start].contains("x")) {
                                    dataVal = "x";
                                }
                                else {
                                    binDataVal = Integer.parseInt(Signal.signalMap.get(item).data[numCell+start], 2);
                                }
                        
                                dataVal = Integer.toHexString(binDataVal);
                                setText(item + ": \t" + (numCell+start >= TIME_WIDTH ? "x" : dataVal));
                                setStyle(null); // Reset style for other items
                            }
                        }
                    }
                };
                // Set preferred height for each cell
                cell.setPrefHeight(cellHeight); // Example height of 30 pixels
                return cell;
            });

        });

        // Create the scene
        Scene scene = new Scene(rootLayout, 800, 600);
        primaryStage.setScene(scene);
        primaryStage.setTitle("Wave Viewer");
        primaryStage.show();
    }


    private void generateWaves() {
        // Clear the wavePane before adding new waves
        wavePane.getChildren().clear();
        // Add waves to the FlowPane
        
        Platform.runLater(() -> {
            Pane scale = generateScale(wavePane.getWidth());
            wavePane.getChildren().add(scale);
        });

        // Add waves to the FlowPane
        for (Signal signal : signalList) {
            String[] data = signal.data;
            Platform.runLater(() -> {
                Pane wave = createWavePane(data, wavePane.getWidth(), signal.singleBit);
                wavePane.getChildren().add(wave);
            });
        }
    }

    private Pane generateScale(double paneWidth) {
        Pane pane = new Pane();
        pane.setPrefHeight(cellHeight);
        for (int i = 0; i < WIDTH_NUMBER; i++) {
            Text text = new Text(TIME_STEP*(i+start)+"");
            text.setLayoutX(i * paneWidth/WIDTH_NUMBER + paneWidth/WIDTH_NUMBER/2);
            text.setLayoutY(cellHeight/1.5);
            text.setStyle("-fx-font-weight: bold; -fx-text-fill: darkblue; -fx-alignment: center; -fx-font-size: 14;");
            text.setFill(Color.DARKBLUE);
            pane.getChildren().add(text);
        }
        Line greenLine = new Line(0, cellHeight, paneWidth, cellHeight);
        greenLine.setStroke(Color.BLACK);
        greenLine.setStrokeWidth(0.2); // Set stroke width to 1 pixel for a thin line
        pane.getChildren().add(greenLine);
        return pane;
    }

    private Pane createWavePane(String[] data, double paneWidth, boolean singleBit) {
        Pane wp = new Pane();
        
        wp.setPrefHeight(cellHeight);
        
        for (int i = 0; i < WIDTH_NUMBER; i++) {
            if ( i +start >= TIME_WIDTH || data[i + start].contains("x")) {
                // Rectangle rect = new Rectangle(i * paneWidth/WIDTH_NUMBER, 0, paneWidth/WIDTH_NUMBER, cellHeight);
                Rectangle rect = new Rectangle(i * paneWidth/WIDTH_NUMBER, 2, paneWidth/WIDTH_NUMBER, cellHeight-4);
                rect.setFill(Color.RED);
                wp.getChildren().add(rect);
            }
            else {
                Line line = new Line(i * paneWidth/WIDTH_NUMBER, (1-1)*(cellHeight)+4, (i + 1) * paneWidth/WIDTH_NUMBER, (1-1)*(cellHeight)+4); // Draw line between points
                line.setStroke(Color.BLUE);
                Line line1 = new Line(i * paneWidth/WIDTH_NUMBER, (1-0)*(cellHeight)-4, (i + 1) * paneWidth/WIDTH_NUMBER, (1-0)*(cellHeight)-4); // Draw line1 between points
                line1.setStroke(Color.BLUE);
                
                if (singleBit) {
                    if (data[i+start].equals("1")) {
                        wp.getChildren().add(line);
                    }
                    else {
                        wp.getChildren().add(line1);
                    }
                }
                else {
                    wp.getChildren().add(line);
                    wp.getChildren().add(line1);
                    int binDataVal = Integer.parseInt(data[i+start], 2);
                    String dataVal = Integer.toHexString(binDataVal);
                    Text text = new Text(dataVal);
                    text.setLayoutX(i * paneWidth/WIDTH_NUMBER + paneWidth/WIDTH_NUMBER/2);
                    text.setLayoutY(cellHeight/1.5);
                    wp.getChildren().add(text);
                }
            }
        }
        for (int i = 0; i < WIDTH_NUMBER-1; i++) {
            if (i + start >= TIME_WIDTH || i + 1 + start >= TIME_WIDTH || data[i + start].contains("x") || data[i + 1 + start].contains("x")) {
                continue;
            }
            if (data[i+start].equals(data[i+start+1])) continue;
            Line line = new Line((i + 1) * paneWidth/WIDTH_NUMBER, (1-1)*(cellHeight)+4, (i + 1) * paneWidth/WIDTH_NUMBER, (1-0)*(cellHeight)-4); // Draw line between points
            line.setStroke(Color.BLUE);
            wp.getChildren().add(line);
        }
        Line greenLine = new Line(0, cellHeight, paneWidth, cellHeight);
        greenLine.setStroke(Color.GREEN);
        greenLine.setStrokeWidth(0.1); // Set stroke width to 1 pixel for a thin line
        wp.getChildren().add(greenLine);
        return wp;
    }

    private void saveState(Stage primaryStage) {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Save File");

        // Add an extension filter
        String currentDir = System.getProperty("user.dir");
        fileChooser.setInitialDirectory(new File(currentDir));

        FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("Text files (*.GTK)", "*.GTK");
        fileChooser.getExtensionFilters().add(extFilter);

        // Show save file dialog
        File file = fileChooser.showSaveDialog(primaryStage);

        if (file != null) {
            saveToFile(file);
        }
    }

    private void saveToFile(File file) {
        try (FileWriter writer = new FileWriter(file)) {
            for (Signal sig : signalList) {
                writer.write(sig.name + "\n");
            }
            System.out.println("File saved: " + file.getAbsolutePath());
        } catch (IOException e) {
            System.err.println("Error saving file: " + e.getMessage());
        }
    }

    private void openState(Stage primaryStage) {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Open File");
    
        // Set initial directory
        String currentDir = System.getProperty("user.dir");
        fileChooser.setInitialDirectory(new File(currentDir));
    
        // Add an extension filter
        FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("Text files (*.GTK)", "*.GTK");
        fileChooser.getExtensionFilters().add(extFilter);
    
        // Show open file dialog
        File file = fileChooser.showOpenDialog(primaryStage);
    
        if (file != null) {
            readFromFile(file);
        }
        
    }

    private void readFromFile(File file) {
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line;
            signalList.clear();
            signalListView.getItems().clear();
            Signal.signalMap.clear();
            // System.out.println("Contents of " + file.getName() + ":");
            signalListView.getItems().add(0, "SIGNAL");

            while ((line = reader.readLine()) != null) {
                System.out.println(line);
                signalList.add(new Signal(line, ALL_SIGNALS.get(line)));
                signalListView.getItems().add(line);
                Signal.signalMap.put(line, new Signal(line, ALL_SIGNALS.get(line)));
            }
            generateWaves();
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
    }

    private MenuBar createMenuBar(Stage primaryStage) {
        // Create the MenuBar
        MenuBar menuBar = new MenuBar();
    
        // Create the File menu
        Menu fileMenu = new Menu("File");
        MenuItem newItem = new MenuItem("New");
        MenuItem openItem = new MenuItem("Open");
        MenuItem saveItem = new MenuItem("Save");
        MenuItem exitItem = new MenuItem("Exit");
        fileMenu.getItems().addAll(newItem, openItem, saveItem, new SeparatorMenuItem(), exitItem);
    
        // Set event handler for the exit item
        exitItem.setOnAction(e -> primaryStage.close());
        saveItem.setOnAction(e -> saveState(primaryStage));
        openItem.setOnAction(e -> openState(primaryStage));
    
        // Create the Help menu
        Menu helpMenu = new Menu("Help");
        MenuItem aboutItem = new MenuItem("About");
        helpMenu.getItems().addAll(aboutItem);
    
    
        // // Create a CustomMenuItem to hold the button
        // CustomMenuItem buttonMenuItem = new CustomMenuItem(forwardButton);
        // buttonMenuItem.setHideOnClick(false); // Prevent the menu from closing when clicking the button
    
        // // Create an empty menu to hold the CustomMenuItem (acting as a placeholder)
        // Menu buttonMenu = new Menu();
        // buttonMenu.getItems().add(buttonMenuItem);
    
        // Add menus and the buttonMenu to the MenuBar
        menuBar.getMenus().addAll(fileMenu, helpMenu);
    
        return menuBar;
    }

    private void addSignal(String sigName) {
        // Add the signal to the signal list
        if (!ALL_SIGNALS.containsKey(sigName)) return;
        Signal signal = new Signal(sigName, ALL_SIGNALS.get(sigName));
        signalList.add(signal);
        // Add the signal name to the ListView
        signalListView.getItems().add(sigName);
        // Generate the waves
        generateWaves();
    }

    private void initializeData() {
        signalList = new ArrayList<>();
        // Construct path to CSV file
        String csvFile = "src/output.csv";
        String line;

        try (BufferedReader br = new BufferedReader(new FileReader(csvFile))) {
            while ((line = br.readLine()) != null) {
                String[] values = line.split(",");
                
                // First column
                String firstColumn = values[0];
                
                // Rest of the columns
                String[] restOfColumns = new String[values.length - 1];
                System.arraycopy(values, 1, restOfColumns, 0, values.length - 1);
                ALL_SIGNALS.put(firstColumn, restOfColumns);
                TIME_WIDTH = Math.max(TIME_WIDTH, restOfColumns.length +1 );
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String[] arraySplicer(String[] arr, int index) {
        String[] narr = new String[arr.length];
        for (int i = 0; i < arr.length; i++) {
            narr[i] = arr[i].equals("x") ? "x" : arr[i].substring(index, index+1);
        }
        return narr;
    }

    private int findDataTimeLength(String[] arr) {
        int len = 0;
        for (String item : arr) {
            if (item == "x") continue;
            len = Math.max(len, (item).length());
        }
        return len;
    }

    private class Signal {
        private String name;
        private String[] data;
        private boolean singleBit;
        private boolean fake;
        private static ArrayList<String> names = new ArrayList<>();
        private static HashMap<String, Signal> signalMap = new HashMap<>();

        public Signal(String name, String[] data) {
            this.name = name;
            this.data = data;
            singleBit = findDataTimeLength(data) == 1;
            names.add(name);
            signalMap.put(name, this);
            fake = false;
        }
    }

    private static class DragAndDropListCell extends ListCell<String> {
        public DragAndDropListCell() {
            setOnDragDetected(event -> {
                if (getItem() == null) {
                    return;
                }

                Dragboard dragboard = startDragAndDrop(TransferMode.MOVE);
                ClipboardContent content = new ClipboardContent();
                content.putString(getItem());
                dragboard.setContent(content);

                event.consume();
            });

            setOnDragOver(event -> {
                if (event.getGestureSource() != this &&
                        event.getDragboard().hasString()) {
                    event.acceptTransferModes(TransferMode.MOVE);
                }

                event.consume();
            });

            setOnDragDropped(event -> {
                if (getItem() == null) {
                    return;
                }

                Dragboard db = event.getDragboard();
                boolean success = false;
                if (db.hasString()) {
                    ObservableList<String> items = getListView().getItems();
                    int draggedIdx = items.indexOf(db.getString());
                    int thisIdx = items.indexOf(getItem());

                    // Swap items
                    String temp = items.get(draggedIdx);
                    items.set(draggedIdx, getItem());
                    items.set(thisIdx, temp);

                    success = true;
                }
                event.setDropCompleted(success);
                event.consume();
            });
        }

        @Override
        protected void updateItem(String item, boolean empty) {
            super.updateItem(item, empty);
            setText(item);
        }
    }   

    public static void main(String[] args) {
        launch(args);
    }
}

/*
 * TODO
 * 1. make sure sizes line up when loading file
 * 2. add naming to add signal
 * 3. fix drag and drop
 */