#include "buffer.h"
#include "InputProcessor.h"
#include "cursor.h"
#include "interface.h"
#include <ncurses.h>
#include <stdio.h>
#include <string>

using namespace std;

InputProcessor::InputProcessor(Interface* interface) : interface{interface} {}

bool InputProcessor::handle_commands(Buffer &b){
     char ch = interface->getChar();

     if(b.mode == NORMAL) {
       bool quit = normalModeInput(b, ch);
       if(quit) {
         return true;
       }
     }
     else if(b.mode == INSERT) insertModeInput(b, ch);

     return false;
}

Command InputProcessor::ex_command_mode(Buffer &b){ 
    int row, col;
    int savedY, savedX;
    getmaxyx(stdscr, row, col);
    interface->getCursorPosition(savedY, savedX);
    char last_line[col+1];
    for(int i=0; i<col; i++){
      last_line[i]=' ';
    }
    last_line[col+1] = '\0';

    row = row-1;
    interface->moveAndReadString(row, 0, last_line);
    clrtoeol();

    interface->moveAndWriteString(row, 0, ":");
    refresh();

    string cmd;
    int cmd_i=0;

    char c;

    while(cmd_i<20 && c != 10){
        c = interface->getChar();
        addch(c);
        refresh();
        if(c != 10) cmd+=c;
        cmd_i++;

    }

    interface->moveAndWriteString(row, 0, last_line);
    refresh();
    interface->move(savedY, savedX);
    try{
      refresh();
    }catch (const std::exception& e){
      printf("%s", e.what());
    }
    if(cmd == "q"){
        return QUIT;
    }
    if(cmd == "wq"){
        b.save();
        return QUIT;
    }
    if(cmd == "w"){
        b.save();
    }
    return UNKNOWN;
}

bool InputProcessor::normalModeInput(Buffer &b, char ch, string state){
    state += ch;
    try{
        if(state == "O"){
            b.insertLineAboveCursor();
            b.moveCursor(UP);
            b.moveCursor(BEGINNING_OF_LINE);
            b.mode = INSERT;
            cursorLine();
        }
        if(state == "o"){
            b.moveCursor(END_OF_LINE);
            b.insertLineAfterCursor();
            b.mode = INSERT;
            cursorLine();
            b.moveCursor(DOWN);
        }
        if(state == "k") moveCursor(b, UP);
        if(state == "j") moveCursor(b, DOWN);
        if(state == "J") b.joinLineAtCursor();
        if(state == "h") moveCursor(b, LEFT);
        if(state == "l") moveCursor(b, RIGHT);
        if(state == "G") moveCursor(b, b.contents.size()-1);
        if(state == "c") cursorUnderscore();
        if(state == "i") {
            b.mode = INSERT;
            cursorLine();
        }
        if(state == "I") {
            b.mode = INSERT;
            b.moveCursor(BEGINNING_OF_LINE);
            cursorLine();
        }
        if(state == "a") {
            b.mode = INSERT;
            b.moveCursor(RIGHT);
            cursorLine();
        }
        if(state == "A"){
            b.mode = INSERT;
            b.moveCursor(END_OF_LINE);
            cursorLine();
        }
        if(state == "x") {
          b.delete_at_cursor();
        }
        if(state == "f") normalModeInput(b, interface->getChar(), state);
        if(state[0] == 'f' && state.length() > 1) b.find_character_forward(state[1]);
        if(state == "F") normalModeInput(b, interface->getChar(), state);
        if(state[0] == 'F' && state.length() > 1) b.find_character_backward(state[1]);
        if(state == "g") normalModeInput(b, interface->getChar(), state);
        if(state == "gg") moveCursor(b, 0);
	if(state == "d") normalModeInput(b, interface->getChar(), state);
        if(state == "dd") b.delete_line();          
        if(state == "p") b.paste_after();
        if(state == ":"){
          Command cmd = ex_command_mode(b);
          if(cmd == QUIT){
            return true;
          }
        }
    } catch(const char* e){
        beep();
    }
    return false;
}

bool InputProcessor::normalModeInput(Buffer &b, char ch){
    return normalModeInput(b, ch, "");
}

void InputProcessor::insertModeInput(Buffer &b, char ch){
    cursorLine();
    switch(ch){
        //Backspace
        case 8:
        case 127:
            if(b.cursorX == 0){
                b.moveCursor(UP);
                b.moveCursor(END_OF_LINE);
                b.joinLineAtCursor();
            } else {
                b.moveCursor(LEFT);
                b.delete_at_cursor();
            }
            break;
        //Escape
        case 27:
            b.mode = NORMAL;
            if (b.cursorX > 0) b.moveCursor(LEFT);
            cursorBlock();
            break;
        //Enter
        case 10:
        case 13:
            b.insertLineAfterCursor();
            b.moveCursor(DOWN);
            b.moveCursor(BEGINNING_OF_LINE);
            break;
        default:
            b.insertAtCursor(ch);
            b.moveCursor(RIGHT);
    }
}
