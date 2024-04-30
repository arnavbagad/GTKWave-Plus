// libc includes (available in both C and C++)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
// C++ stdlib includes (not available in C)
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

char  *  program;
char  * current;
std::unordered_map<std::string, std::unordered_set<std::string>> dependency_table{};
std::unordered_map<std::string, bool> is_wire{};
std::unordered_set<std::string> modules{};
std::unordered_set<std::string> inters{};
[[noreturn]]
void fail() {
    printf("failed at offset %ld\n",size_t(current-program));
    printf("%s\n",current);
    exit(1);
}
void end_or_fail() {
    while (isspace(*current)) {
      current += 1;
    }

    if (*current != 0) fail();
}

bool consume(const char* str);

void consume_or_fail(const char* str) {
    if (!consume(str)) {
        fail();
    }
}

void skip() {
    while (isspace(*current)) {
        current += 1;
    }
}

bool consume(const char* str) {
    skip();

    size_t i = 0;
    while (true) {
        char const expected = str[i];
        char const found = current[i];
        if (expected == 0) {
            /* survived to the end of the expected string */
            current += i;
            return true;
        }
        if (expected != found) {
        return false;
        }
        // assertion: found != 0
        i += 1;
    }
}

std::optional<std::string> consume_identifier() {
    skip();
    std::string ret = "";

    if (isalpha(*current) || *current == '_') {
        do {
            ret += *current;
            current++;
        } while(isalnum(*current)|| *current == '_');
        return ret;
    } else {
        return {};
    }
}

std::optional<uint64_t> consume_literal() {
    skip();

    if (isdigit(*current)) {
        uint64_t v = 0;
        do {
        v = 10*v + ((*current) - '0');
        current += 1;
        } while (isdigit(*current));
        return v;
    } else {
        return {};
    }
}

void expression(std::string left) {
    //remove all functionality except for extracting identifiers. Keep searching until semicolon.
    while (*current != ';') {
        if (consume("$")) {
            consume_identifier(); //write, readmem, signed, etc.
            consume("(");
            size_t cnt = 1;
            while (cnt) {
                if (*current=='(') cnt++;
                else if (*current==')') cnt--;
                current++;
            }
            continue;
        }
        bool is_num = false;
        if (current[-1] == '\'') is_num = true;
        auto id = consume_identifier();
        if (id && !is_num) {

            dependency_table[left].insert(id.value());
        }
        else {
            current++;
        }
    }
    current++;

}
void statements();

bool statement() {
    skip();
    if (current[0] == 'e' && current[1] == 'n' && current[2] == 'd') return false;



    if (consume("//")) {
        while (*current != '\n') current++;
        current++;
        return statement();
    } else if (consume("/*")) {
        while (!consume("*/")) {
            current++;
        }
        return statement();
    } else if (consume("$")) {
        consume_identifier(); //write, readmem, signed, etc.
        consume("(");
        size_t cnt = 1;
        while (cnt) {
            if (*current=='(') cnt++;
            else if (*current==')') cnt--;
            current++;
        }
        consume(";");
        return true;
    } else if (consume("`")) { //compiler message (timescale)
        while (*current != '\n') current++;
        current++;
        return true;
    } else if (consume("wire")) {
        // wire varName;
        // wire varName = expression;
        // wire [x:y] varName;
        // wire [x:y] varName = expression;
        
        // TODO: Implement this

        if (consume("[")) {
            while (*current != ']') current++;
            current++;
        }
        auto id = consume_identifier();
        is_wire[id.value()] = 1;
        if (consume("=")) expression(id.value());
        else consume(";");
        return true;
    } else if (consume("reg ")) {
        // reg varName;
        // reg varName = expression;
        // reg [x:y] varName;
        // reg [x:y] varName = expression;
        // reg [x:y] varName [a:b];
        // reg [x:y] varName [a:b] = expression;

        // TODO: Implement this

        
        if (consume("[")) {
            while (*current != ']') current++;
            current++;
        }
        auto id = consume_identifier();
        is_wire[id.value()] = 0;
        if (consume("[")) {
            while (*current != ']') current++;
            current++;
        }
        if (consume("=")) expression(id.value());
        else consume(";");
        return true;
        
    } else if (consume("module")) {
        // module varName(params);
        // TODO: Implement this
        // Consume everything until find a semicolon
        while (!isalpha(*current)) current++;
        modules.insert(consume_identifier().value());
        while (*current != ';') current += 1;
        current++;
        statements();
        consume("endmodule");
        return true;

    } else if (consume("assign")) {
        if (consume("[")) {
            while (*current != ']') current++;
            current++;
        }
        auto id = consume_identifier();
        consume("=");
        expression(id.value());
        return true;

    } else if (consume("integer")) {
        auto id = consume_identifier();
        inters.insert(id.value());
        consume("=");
        expression(id.value());
        return true;
    } else if (consume("if")) {
        if (consume("(")) {
            size_t cnt = 1;
            while (cnt) {
                if (*current=='(') cnt++;
                else if (*current==')') cnt--;
                current++;
            }
        }
        if (consume("begin")) {
            statements();
            consume("end");
        }
        else statement();
        return true;

    } else if (consume("else")) {
        if (consume("begin")) {
            statements();
            consume("end");
        }
        else statement();
        return true;
    } else if (consume("always")) {
        consume("@");
        if (consume("(")) {
            size_t cnt = 1;
            while (cnt) {
                if (*current=='(') cnt++;
                else if (*current==')') cnt--;
                current++;
            }
        }
        if (consume("begin")) {
            statements();
            consume("end");
        }
        else statement();
        return true;
    }else if (consume("initial")) {
        consume("@");
        if (consume("(")) {
            size_t cnt = 1;
            while (cnt) {
                if (*current=='(') cnt++;
                else if (*current==')') cnt--;
                current++;
            }
        }
        if (consume("begin")) {
            statements();
            consume("end");
        }
        else statement();
        return true;
    } else if (auto id = consume_identifier()) {
        // reg1 <= ...
        if (consume("<=")) {
            expression(id.value());
            return true;
        }
        //module, etc.
        else {
            while (*current != ';') current += 1;
            current++;
            return true;
        }
    }
    return false;

}

void statements() {
    while (statement());
}

void run() {
    statements();
    end_or_fail();
}

std::unordered_set<std::string> vis;
std::unordered_set<std::string> on_stack;
std::vector<std::string> cycle;

std::unordered_map<std::string, std::unordered_set<std::string>> in_edges;
std::unordered_map<std::string, std::unordered_set<std::string>> out_edges;

bool dfs(std::string str) {
    vis.insert(str);
    on_stack.insert(str);
    for (std::string next : out_edges[str]) {
		if (on_stack.find(next) != on_stack.end()) {
			cycle.push_back(str);  // start cycle
            on_stack.erase(str);
            on_stack.erase(next);
			return true;
		} else if (vis.find(next) == vis.end()) {
			if (dfs(next)) {  // continue cycle
				if (on_stack.find(str) != on_stack.end()) {
					cycle.push_back(str);
					on_stack.erase(str);
					return true;
				} else {  // found u again
					cycle.push_back(str);
					return false;
				}
			}

			if (!cycle.empty()) {
				return false;  // finished with cycle
			}
		}
	}
    on_stack.erase(str);
    return false;
}


int main(int argc, const char *const *const argv) {
    if (argc != 2) {
        fprintf(stderr,"usage: %s <file name>\n",argv[0]);
        exit(1);
    }

    // open the file
    int fd = open(argv[1],O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // determine its size (std::filesystem::get_size?)
    struct stat file_stats;
    int rc = fstat(fd,&file_stats);
    if (rc != 0) {
        perror("fstat");
        exit(1);
    }

    // map the file in my address space
    program = (char *)mmap(0,file_stats.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    current = program;
    
    if (program == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    run();
    std::cout << "VARIABLE NAME: DEPENDENCIES\n";
    for (auto pair : dependency_table) {
        std::cout << pair.first << ": ";
        for (auto dep : pair.second)
            std::cout << dep << " ";
        std::cout << std::endl;
    }
    for (auto pair : is_wire) {
        if (dependency_table.find(pair.first) == dependency_table.end()) {
            std::cout << pair.first << ": \n";
        }
    }



    for (auto pair : dependency_table) {
        //we only need to check pairs of wires, assuming registers are updated per cycle and not continuously.
        if (!is_wire[pair.first]) continue;
        for (auto p2 : pair.second) {
            if (!is_wire[p2]) continue;
            in_edges[pair.first].insert(p2);
            out_edges[p2].insert(pair.first);
        }
    }
    
    for (auto pair : in_edges) {
        if (!cycle.empty()) break;
        dfs(pair.first);
    }

    if (!cycle.empty()) {
        std::cout << "CIRCULAR DEPENDENCY" << std::endl;
		reverse(cycle.begin(), cycle.end());
		for (std::string str : cycle) {
            std::cout << str << " ";
        }
		std::cout << cycle[0] << std::endl;
	}

}
