#include <iostream>
#include <list>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LIMIT 100

using std::cout;
using std::cerr;
using std::cin;

using std::endl;
using std::getline;

using std::string;
using std::list;

int main() {
    string command;
    bool stop = false;
    while(!stop) {
        cout << "CS347(M)$ ";
        getline(cin, command);
        if (command == "exit") {
            stop = true;
        } else if (command == "name") {
            cout << "Kalpesh Krishna & Kumar Ayush" << endl;
        } else if (command == "roll") {
            cout << "140070017 & 140260016" << endl;
        } else {
            // Used to parse the string on spaces
            std::istringstream iss(command);
            char* tokens[LIMIT];
            string token;
            int counter = 0;
            // Parsing command and dumping to `tokens`
            while (!iss.eof()) {
                iss >> token;
                tokens[counter] = new char(token.length() + 1);
                strcpy(tokens[counter], token.c_str());
                counter++;
            }
            tokens[counter] = NULL;
            // System calls begin
            int pid = fork();
            int status = 0;
            if (pid < 0) {
                cerr << "Process could not be created!" << endl;
                exit(1);
            } else if (pid == 0) {
                int exec_ret = execvp(tokens[0], tokens);
                cerr << "Command failed to execute!" << endl;
                exit(1);
            } else {
                wait(&status);
                // Removing tokens from heap
                for (int i = 0; i < counter; i++) {
                    delete[] tokens[i];
                }
            }
        }
    }
}
