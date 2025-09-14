// Identifier: A8A3A33EF075ACEF9B08F5B9845569ECCB423725
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <stack>
#include <getopt.h>

using namespace std;

void printHelp(char *command) {
    cout << "Usage: " << command << " {OPTIONS}\n";
    cout << "OPTIONS:\n";
    cout << "-h, --help                   prints help message";
    cout << "-q, --queue                  search container = queue, uses breadth first search\n";
    cout << "-s, --stack                  search container = stack, uses depth first search\n";
    cout << "-o {TYPE}, --output {TYPE}   specifies output type, requires argument {TYPE} = 'map' or 'list'\n" << flush;
}

enum class SearchMode {
    kNone,
    kQueue,
    kStack,
};

enum class OutputType {
    kMap,
    kList,
};

struct PuzzleOptions {
    SearchMode search_mode = SearchMode::kNone;
    OutputType output_type = OutputType::kMap;
    // string input_file; ????
};

void getOptions(int argc, char **argv, PuzzleOptions &options) {

    struct option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"queue", no_argument, nullptr, 'q'},
            {"stack", no_argument, nullptr, 's'},
            {"output", required_argument, nullptr, 'o'},
            { nullptr, 0, nullptr, '\0' }
        };

    int choice = 0;
    int option_index = 0;

    while ((choice = getopt_long(argc, argv, "hqso:", long_options, &option_index)) != -1) {
        switch(choice) {
            
            case 'h':
                printHelp(* argv);
                exit(0);

            case 'q':
                if (options.search_mode != SearchMode::kNone) { 
                    cerr << "Error: Cannot specify both --queue and --stack\n";
                    exit(1);
                }
                options.search_mode = SearchMode::kQueue;
                break;

            case 's':
                if(options.search_mode != SearchMode::kNone) {
                    cerr << "Error: Cannot specify both --queue and --stack\n";
                    exit(1);
                }
                options.search_mode = SearchMode::kStack;
                break;

            case 'o': { // Need a block here to declare a variable inside a case
                string output_arg{optarg}; // optarg automatically provided by <getopt.h>
                if(output_arg == "map") {
                    options.output_type = OutputType::kMap;
                } else if (output_arg == "list") {
                    options.output_type = OutputType::kList;
                } else {
                    cerr << "Error: --output must be 'map' or 'list'\n";
                    exit(1);
                }
                break;
            }

            default:
                cerr << "Error: invalid option\n" << flush;
                exit(1);
        }
    }
// if(optind < argc) { options.input_file = argv[optind]; }
    if (options.search_mode == SearchMode::kNone) {
        cerr << "Error: no search mode specified\n" << flush;
        exit(1);
    }
}


uint32_t charToNum(char c) { 
    if (c == '^') return 0;
    if (isupper(c)) return static_cast<uint32_t>(c - 'A' + 1);
    if (islower(c)) {
        uint32_t num = static_cast<uint32_t>(c - 'a' + 1);
        if (num > 26) {  // Safety check
            cerr << "Invalid color character: '" << c << "'\n";
            exit(1);
        }
        return num;
    }
    cerr << "Invalid color character: '" << c << "'\n";
    exit(1);
}

char numToChar(uint32_t num) {
    if (num == 0) return '^';
    if (num >= 1 && num <= 26) return 'a' + static_cast<char>(num - 1);
    if (num == 27) return 'z';
    cerr <<"Invalid number " << num << " \n"; 
    exit(1);
}



struct Coord {
    uint32_t color;
    uint32_t row;
    uint32_t column;

    // Initializer
     Coord(uint32_t c, uint32_t r, uint32_t cl) : color(c), row(r), column(cl) {};

};

struct PuzzleMap {
    uint32_t num_colors = 0;
    uint32_t height = 0;
    uint32_t width = 0;

    vector<vector<char>> grid;
    Coord start{charToNum('^'),0,0}, target{charToNum('^'),0,0}; // Used {} to initialize bc I'm initializing in a struct!
};

bool isValidChar(char c, uint32_t num_colors) {
    if (c == '@' || c == '#' || c == '^' || c == '.' || c == '?') {
        return true;
    } else if (isupper(c)) { 
        return (static_cast<uint32_t>(c-'A') <= (num_colors));
    } else if (islower(c)) {
        return (static_cast<uint32_t>(c-'a') <= (num_colors));
    } else {return false;}
}


PuzzleMap readInput() {

// Reading in map parameters
    PuzzleMap map;
    if(!(cin >> map.num_colors >> map.height >> map.width)) {
        cerr << "Error: must specify num_colors, height, width!\n";
        exit(1);
    }
// Checking valid map parameters
    if (!(map.num_colors <= 26)) {
        cerr << "Error: must have 0 <= num_colors <= 26\n";
        exit(1);
    }
    if (map.height < 1 || map.width < 1) {
        cerr << "Error: height and width must be >= 1\n";
        exit(1);
    }
// Resizing
    map.grid.resize(map.height, vector<char>(map.width, '.'));

// Ignoring comments
    string junk;
    while(getline(cin, junk)) {
        if ((junk.substr(0, 2) == "//") || junk.empty()) {
            continue; 
        } else {break;}
    }
// Track number of starts and targets read
uint32_t num_starts =0, num_targets =0;
string line = junk;

// For each row/line
    for(uint32_t i=0; i < map.height; ++i) {
// For each char in line
        for (uint32_t j = 0; j < map.width; ++j) {
            char c = line[j];

// Check valid character and add to map
            if (!isValidChar(c, map.num_colors)) {
                cerr << "Error: Invalid char '" << c 
                          << "' in line " << line << "\n";
                exit(1);
            } else {map.grid[i][j] = c;}

// Track start/target
            if (c == '@') {
                map.start = Coord(charToNum('^'),i,j);
                num_starts++;
            } else if (c == '?') {
                map.target = Coord(charToNum('^'),i,j);
                num_targets++;
            }
        }
// Get the next line
        getline(cin, line);
    }

// Validate start/target counts at the end
    if (num_starts != 1 || num_targets != 1) {
        cerr << "Error: Missing/excess '@' or '?'\n";
        exit(1);
    }
    return map;
}
















// Initialize a bactrace map
vector<vector<vector<char>>> createBacktraceMap (PuzzleMap const & map) {
    vector<vector<vector<char>>> backtrace (map.num_colors + 1, // vector contains number of grids
                vector<vector<char>>(map.height,  // vector contains number of rows
                vector<char>(map.width, '-'))); // charatcers in each row-vector preset to '-'

    
    return backtrace;
}





void generateListOutput(Coord target_state, vector<vector<vector<char>>> const &bt) {
    stack<Coord> path;
    Coord current = target_state;
    path.push(current);
// While not start, look at marker in the backtrace
    while (bt[current.color][current.row][current.column] != '@') {
        char marker = bt[current.color][current.row][current.column];
        if (islower(marker) || marker == '^') {
            current = Coord{charToNum(marker), current.row, current.column};
        } else {
            switch (marker) {
                case 'N': current = Coord{current.color, current.row - 1, current.column}; break;
                case 'E': current = Coord{current.color, current.row, current.column + 1}; break;
                case 'S': current = Coord{current.color, current.row + 1, current.column}; break;
                case 'W': current = Coord{current.color, current.row, current.column - 1}; break;
                default:
                    cerr << "Error: Invalid path marker\n";
                    exit(1);
            }
        }

        path.push(current);
    }

    // Print path from start to goal
    while (!path.empty()) {
        Coord c = path.top();
        path.pop();
        cout << "(" << numToChar(c.color) << ", (" << c.row << ", " << c.column << "))\n";
    }
}

void generateMapOutput(PuzzleMap const &map, Coord const &solution_state, 
                      vector<vector<vector<char>>> const &btrace) {
// Path reconstruction 
    stack<Coord> path;
    Coord current = solution_state;
    path.push(current);

    while (btrace[current.color][current.row][current.column] != '@') {
    // Get marker from backtrace
        char marker = btrace[current.color][current.row][current.column];
        
    // Change color if marker = button
        if (islower(marker) || marker == '^') {
            current = Coord{charToNum(marker), current.row, current.column};
        } else {
    // Move if marker = direction
            switch (marker) {
                case 'N': current = Coord{current.color, current.row - 1, current.column}; break;
                case 'S': current = Coord{current.color, current.row + 1, current.column}; break;
                case 'E': current = Coord{current.color, current.row, current.column + 1}; break;
                case 'W': current = Coord{current.color, current.row, current.column - 1}; break;
                default:
                    cerr << "Error: Invalid path marker\n";
                    exit(1);
            }
        }
        path.push(current);
    }

// 2 3D vectors filled with FALSE
    vector<vector<vector<bool>>> is_on_path(
        map.num_colors + 1, 
        vector<vector<bool>>(map.height, 
        vector<bool>(map.width, false)));


// While loop to fill is_on_path
    while (!path.empty()) {
        Coord c = path.top();
        path.pop();
        is_on_path[c.color][c.row][c.column] = true;
    }

// Generate each color map
    for (uint32_t color = 0; color <= map.num_colors; ++color) {
        cout << "// color " << numToChar(color) << "\n";
        for (uint32_t row = 0; row < map.height; ++row) {
            for (uint32_t col = 0; col < map.width; ++col) {
                char original = map.grid[row][col];
                char output = original;
            // If map coord on solution path, mark '+' or smth
                if (is_on_path[color][row][col]) {
                    if(original == '@' && color != 0) {output = '+';}
                    if (original == '.' || (original == toupper(numToChar(color)) && original != '^')) {
                        output = '+';
                // If button
                    } else if (islower(original)) {
                        //if color of current color map = button color  AND  backtrace is marked with a button
                        if (color == charToNum(original)) {
                            // if button character is different from backtrace character at same color,row,column, then @
                            if (btrace[color][row][col] != original) {
                                output = '@';
                            } else {output = '+';} // otherwise if button character N,S,E,W, then +
                            
                        } else {
                            output = '%';
                        }
                    }
                    // Keep @ for start (only on color ^)
                    if (original == '@' && color == 0) {
                        output = '@';//
                    }
                    // Always show target
                    if (original == '?') {
                        output = '?';
                    }
            /////////////////////////////////////
                // handle trapped buttons
                    if (original == '^') {
                        if (color == charToNum('^')) {
                            // Only show @ if actually pressed the trap
                            if (islower(btrace[color][row][col])) {
                                output = '@';
                            } else {
                                output = '+';
                            }

                        } else {
                            output = '%';
                        }
                    }
            //////////////////////////////////////
                }
                else {
                        if(original == '@' && color != 0) {output = '@';}
                    // Not on solution path - replace buttons and doors with '.'
                        if ((islower(original) || original == '^') && charToNum(original) == color) {
                            output = '.';
                        } else if (isupper(original) && charToNum(original) == color) {
                            output = '.';
                        } else if(original == '@') {
                            output = '.';
                        }
                }
                cout << output;
            }
            cout << "\n";
        }
    }
}



void printNoSolutionOutput(const PuzzleMap& map, const vector<vector<vector<char>>>& btrace) {
    cout << "No solution.\n";
    cout << "Discovered:\n";
    
    // Print the map with undiscovered locations as '#'
    for (uint32_t row = 0; row < map.height; ++row) {
        for (uint32_t col = 0; col < map.width; ++col) {
            bool discovered = false;
            
            // Check if this location was discovered in any color
            for (uint32_t color = 0; color < btrace.size(); ++color) {
                if (btrace[color][row][col] != '-') {
                    discovered = true;
                    break;
                }
            }
            
            // Print original character if discovered, '#' otherwise
            if (discovered) {
                cout << map.grid[row][col];
            } else {
                cout << '#';
            }
        }
        cout << '\n';
    }
}



void solvePuzzle(vector<vector<vector<char>>>& btrace, PuzzleMap const &map, deque<Coord> sc, PuzzleOptions const &options) {
    // Initialize start
    sc.push_back(map.start);
    btrace[0][map.start.row][map.start.column] = '@';
    Coord current_state = map.start;

    bool solution_found = false;
    Coord solution_state = map.start;

    // While loop
    while(!sc.empty()) {
        if (options.search_mode == SearchMode::kQueue) {
            current_state = sc.front();
            sc.pop_front();
        } else {
            current_state = sc.back();
            sc.pop_back();
        }

        // Check if active button
        char b = map.grid[current_state.row][current_state.column];
        
        // If its a button/^ AND we're in a color state other than button color
        if ((islower(b) || b == '^') && (charToNum(b) != current_state.color)) {
            uint32_t new_color = charToNum(b);
            // If where the button leads to is undiscovered
            if (btrace[new_color][current_state.row][current_state.column] == '-') {
                // Add that undiscovered spot to search container
                Coord button_state{new_color, current_state.row, current_state.column};
                btrace[new_color][current_state.row][current_state.column] = numToChar(current_state.color);
                sc.push_back(button_state);

                    if (b == '?') {
                        solution_found = true;
                        solution_state = button_state;
                        break;
                    }
            }
            continue;  // Skip adjacent checks after button press
        }
        // Check finish
        else if (b == '?') {
            solution_found = true;
            solution_state = current_state;
            break;
        } 
        // Check adjacent locations
        else {
            // Determine if doors should be closed (only if current cell is ^)
            bool trap_doors_closed = (b == '^');
            
            // North
            if(current_state.row > 0) {
                if(btrace[current_state.color][current_state.row-1][current_state.column] == '-') {
                    char n = map.grid[current_state.row-1][current_state.column];
                    if ((n == '.') || (islower(n)) || 
                        (n == toupper(numToChar(current_state.color)) && !trap_doors_closed) || 
                        (n == '^') || 
                        (n == '?') || ((n == '@') && current_state.color != 0)) {
                        Coord north{current_state.color, current_state.row-1, current_state.column};
                        btrace[north.color][north.row][north.column] = 'S';
                        sc.push_back(north);

                        if (n == '?') {
                            solution_found = true;
                            solution_state = north;
                            break;
                        }
                    }
                }
            }
            // East
            if(current_state.column < map.width-1) {
                if(btrace[current_state.color][current_state.row][current_state.column+1] == '-') {
                    char e = map.grid[current_state.row][current_state.column+1];
                    if ((e == '.') || (islower(e)) || 
                        (e == toupper(numToChar(current_state.color)) && !trap_doors_closed) || 
                        (e == '^') || 
                        (e == '?') || ((e == '@') && current_state.color != 0)) {
                        Coord east{current_state.color, current_state.row, current_state.column+1};
                        btrace[east.color][east.row][east.column] = 'W';
                        sc.push_back(east);

                        if (e == '?') {
                            solution_found = true;
                            solution_state = east;
                            break;
                        }

                    }
                }
            }
            // South 
            if(current_state.row < map.height-1) {
                if(btrace[current_state.color][current_state.row+1][current_state.column] == '-') {
                    char s = map.grid[current_state.row+1][current_state.column];
                    if ((s == '.') || (islower(s)) || 
                        (s == toupper(numToChar(current_state.color)) && !trap_doors_closed) || 
                        (s == '^') || 
                        (s == '?') || ((s == '@') && current_state.color != 0)) {
                        Coord south{current_state.color, current_state.row+1, current_state.column};
                        btrace[south.color][south.row][south.column] = 'N';
                        sc.push_back(south);

                        if (s == '?') {
                            solution_found = true;
                            solution_state = south;
                            break;
                        }
                    }
                }
            }
            // West 
            if(current_state.column > 0) {
                if(btrace[current_state.color][current_state.row][current_state.column-1] == '-') {
                    char w = map.grid[current_state.row][current_state.column-1];
                    if ((w == '.') || (islower(w)) || 
                        (w == toupper(numToChar(current_state.color)) && !trap_doors_closed) || 
                        (w == '^') || 
                        (w == '?') || ((w == '@') && current_state.color != 0)){
                        Coord west{current_state.color, current_state.row, current_state.column-1};
                        btrace[west.color][west.row][west.column] = 'E';
                        sc.push_back(west);

                        if (w == '?') {
                            solution_found = true;
                            solution_state = west;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Output handling 
    if(solution_found) {
        if (options.output_type == OutputType::kList) {
            generateListOutput(solution_state, btrace);
        }
        else if (options.output_type == OutputType::kMap) {
            generateMapOutput(map, solution_state, btrace);
        }
    } 
    else if (sc.empty()) {
        printNoSolutionOutput(map, btrace);
    }
}









int main(int argc, char * argv[]) {
    ios_base::sync_with_stdio(false);
    PuzzleOptions options;

// Get options
    getOptions(argc, argv, options);
// Read in map
    PuzzleMap map = readInput();

// make backtrace 3D Vector
    vector<vector<vector<char>>> btrace = createBacktraceMap (map);
// make search_container deque
    deque<Coord> search_container;

// Find map solution and generate output
    solvePuzzle(btrace, map, search_container, options);


}