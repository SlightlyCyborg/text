#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <regex>
#include <sstream>      // std::stringstream


#include "buffer.h"

using namespace std;

int Buffer::findBeginningOfLine(int lineNo){
    string line = contents[lineNo];
    smatch match;
    regex r("\\S");
    regex_search(line, match, r);
    if(match.empty()) return 0;
    return match.position();
}

int Buffer::findEndOfLine(int lineNo){
    string line = contents[lineNo];
    smatch match;
    regex r(".*\\S");
    regex_search(line, match, r);
    if(match.empty()) return 0;
    int start = match.position();
    return start + match[0].length();
}

Buffer::Buffer(string contents){
    initContents(contents);
}

Buffer::Buffer(char* fname){
    ifstream in(fname);
    string raw = string(std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>());
    initContents(raw);
}

void Buffer::deleteAtCursor(){
    if(cursorY < contents.size() &&
            cursorX < contents[cursorY].size()){
        contents[cursorY].erase(cursorX, 1);
        contentsChangedB = true;
    }
}

void Buffer::initContents(string raw){
    string lineBuffer;
    stringstream ss(raw);
    while (getline(ss, lineBuffer)){
        contents.push_back(string(lineBuffer));
    }
}

void Buffer::insertAtCursor(char ch){
    bool lineExists = cursorY < contents.size();
    //colExists uses <= because cursor could insert at end.
    bool colExists  = cursorX <= contents[cursorY].size();
    if(lineExists && colExists){
        contents[cursorY].insert(cursorX, 1, ch);
    }
    contentsChangedB = true;
}

string Buffer::toString(int offset, int numRows){
    string rv = string();
    if(numRows+offset > contents.size()){
        numRows = contents.size()-offset;
    }
    for(int i=offset; i<numRows+offset; i++){
        if(i>offset && i<contents.size()){
            rv.append("\n");
        }
        rv.append(contents[i]);
    }
    return rv;
}

string Buffer::toString(){
    return toString(0, contents.size());
}

void Buffer::resetXCursorToCacheIfNeeded(){
    if(cursorXReset > 0){
        cursorX = cursorXReset;
        cursorXReset = -1;
    }
}

void Buffer::moveXCursorToLineEndAndCacheIfNeeded(){
    //move left if line not long enough
    if(cursorX >= contents[cursorY].size()){
        cursorXReset = cursorX;
        cursorX = contents[cursorY].size()-1;
        if(cursorX < 0) cursorX = 0;
    }
}

int Buffer::cursorXBound(int lineNo){
    int bound = contents[lineNo].size();
    if(mode == NORMAL)
        //Should be able to insert at line.size(), but not scroll to it.
        bound--;
    return bound;
}

void Buffer::moveCursor(Direction d, int amount){
    switch(d){
        case UP:
            if(cursorY - amount < 0)
                throw "can not move cursor up";
            cursorY -= amount;
            resetXCursorToCacheIfNeeded();
            moveXCursorToLineEndAndCacheIfNeeded();
            break;
        case DOWN:
            if(cursorY + amount >= contents.size())
                throw "can not move cursor dwn";
            cursorY += amount;
            resetXCursorToCacheIfNeeded();
            moveXCursorToLineEndAndCacheIfNeeded();
            break;
        case LEFT:
            if(cursorX - amount < 0)
                throw "can not move cursor left";
            cursorX -= amount;
            cursorXReset = -1; //sentinel
            break;
        case RIGHT:
            if(cursorX + amount > cursorXBound(cursorY))
                throw "can not move cursor right";
            cursorX += amount;
            cursorXReset = -1; //sentinel
            break;
        case BEGINNING_OF_LINE:
            cursorX = findBeginningOfLine(cursorY);
            cursorXReset = -1; //sentinel
            break;
        case END_OF_LINE:
            cursorX = findEndOfLine(cursorY);
            cursorXReset = -1; //sentinel
            break;
        default:
            break;
    }
}

void Buffer::moveCursor(int row){
    if(row >= 0 && row < contents.size()){
        cursorY = row;
    }
}

