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

// Implementation includes
#include "slice.h"

class Interpreter {
    char const * const program;
    char const * current;
    std::unordered_map<Slice,uint64_t> symbol_table{};

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

  std::optional<Slice> consume_identifier() {
    skip();

    if (isalpha(*current)) {
      char const * start = current;
      do {
        current += 1;
      } while(isalnum(*current));
      return Slice{start,size_t(current-start)};
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

public:
    Interpreter(char const* prog): program(prog), current(prog) {}

    // The plan is to honor as many Verilog operators as possible
    // e<n> implements operators with precedence 'n' (smaller is higher)

    // () [] i.e. getting bits from a wire, array indexing
    uint64_t e1(bool effects) {

        //add here: consuming $ for $write, $readmem, $signed.

        //TODO: Handle arrays as different identifiers
        if (auto id = consume_identifier()) { 
            auto v = symbol_table[id.value()];


            if (consume("[")) {
            
                auto v = expression(effects);

                    if (consume(":")) {
                        //get other end of bits to consider
                        auto v2 = expression(effects);
                    }

                consume("]");
                //TODO: either get the bit values or array values
                //for bit value need [value : value]
                //for array indexing just [value]
            }

            return v;
        } else if (auto v = consume_literal()) {
            return v.value(); //TODO: Make literals also take binary, hex, oct
        } else if (consume("(")) {
            auto v = expression(effects);
            consume(")");
            return v;
            
        } else {
            //TODO: need to change this to returning null for unary operators instead of fail.
            //i.e. null!A, null~A => !A, ~A (just complement of A)
            fail();
        }
    }

    // &A ~&A |A ~|A ^A ~^A (Reduction, not implementing)
    // !A, ~A, +A, -A (Complement, Unary operators)
    uint64_t e2(bool effects) {
        auto v = e1(effects);

        //v must be null, otherwise return v.
        if (v != NULL) return v; 
        while (true) {
            if (consume("!")) {
                v = e1(effects) != 0 ? 0 : 1;
            } else if (consume("~")) {
                v = ~e1(effects);
            } else if (consume("+")) {
                auto right = e1(effects);
                v = right < 0 ? -right : right;
            } else if (consume("-")) {
                auto right = e1(effects);
                v = right > 0 ? -right : right;
            } 
            else {
                return v;
            }
        }

    }

    // {N{A}}, {A, B, ..., C} => We need to move comma to e15, and make {} run all ex again.
    uint64_t e3(bool effects) {
        auto v = e2(effects);
        //v is the number of times to repeat. 1 = don't repeat

        
        return v;
    }

    // (Left) * / %
    uint64_t e4(bool effects) {
        auto v = e3(effects);

        while (true) {
            if (consume("*")) {
                v = v * e3(effects);
            } else if (consume("/")) {
                auto right = e3(effects);
                v = (right == 0) ? 0 : v / right;
            } else if (consume("%")) {
                auto right = e3(effects);
                v = (right == 0) ? 0 : v % right;
            } else {
                return v;
            }
        }


    }

    // (Left) + -
    uint64_t e5(bool effects) {
        auto v = e4(effects);
        while (true) {
            if (consume("+")) {
                v = v + e4(effects);
            } else if (consume("-")) {
                auto right = e4(effects);
                v = v - e4(effects);
            } else {
                return v;
            }
        }
    }

    // <<, >>
    uint64_t e6(bool effects) {
        auto v = e5(effects);
        while (true) {
            if (consume("<<")) {
                v = v << e5(effects);
            } else if (consume(">>")) {
                v = v >> e5(effects);
            } else {
                return v;
            }
        }
    }

    // A>B A<B A>=B A<=B
    uint64_t e7(bool effects) {
        return e6(effects);
    }

    // A==B A!=B
    uint64_t e8(bool effects) {
        return e7(effects);
    }

    // A&B

    uint64_t e9(bool effects) {
        return e8(effects);
    }

    // A^B A~^B

    uint64_t e10(bool effects) {
        return e9(effects);
    }

    // A|B
    uint64_t e11(bool effects) {
        return e10(effects);
    }

    // A&&B
    uint64_t e12(bool effects) {
        return e11(effects);
    }

    // A||B
    uint64_t e13(bool effects) {
        return e12(effects);
    }

    // ?:
    uint64_t e14(bool effects) {
        return e13(effects);
    }

    // ,
    uint64_t e15(bool effects) {
        return e14(effects);
    }

    uint64_t expression(bool effects) {
        return e15(effects);
    }

    bool statement(bool effects) {
        if (consume("wire")) {
            // wire varName;
            // wire varName = expression;
            // wire [x:y] varName;
            // wire [x:y] varName = expression;
            
            // TODO: Implement this

            /*
            auto id = consume_identifier();
            auto v = expression(effects);
            if (effects) {
                printf("%ld\n",v);
            }
            return true;
            */
        } else if (consume("reg")) {
            // reg varName;
            // reg varName = expression;
            // reg [x:y] varName;
            // reg [x:y] varName = expression;
            // reg [x:y] varName [a:b];
            // reg [x:y] varName [a:b] = expression;

            // TODO: Implement this

            /*
            auto id = consume_identifier();
            auto v = expression(effects);
            if (effects) {
                printf("%ld\n",v);
            }
            return true;
            */
        } else if (consume("module")) {
            // module varName(params);
            // reg varName = expression;
            // reg [x:y] varName;
            // reg [x:y] varName = expression;
            // reg [x:y] varName [a:b];
            // reg [x:y] varName [a:b] = expression;

            // TODO: Implement this

            /*
            auto id = consume_identifier();
            auto v = expression(effects);
            if (effects) {
                printf("%ld\n",v);
            }
            return true;
            */
        } else if (auto id = consume_identifier()) {
            // x = ...
            if (consume("=")) {
                auto v = expression(effects);
                if (effects) {
                    symbol_table[id.value()] = v;
                }
                return true;
            } else {
                fail();
            }
        }
        return false;
    }

    void statements(bool effects) {
        while (statement(effects));
    }

    void run() {
        statements(true);
        end_or_fail();
    }
};


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
    char const* prog = (char const *)mmap(
        0,
        file_stats.st_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0);
    if (prog == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    Interpreter x{prog};

    
    x.run();
    
    
    return 0;
}

// vim: tabstop=4 shiftwidth=2 expandtab