
#ifndef Environment_H
#define Environment_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Environment{
private:
// arrays for ATOMS
std::vector<std::string> Aatom_name;
std::vector<std::string> Aresidue;
//std::vector<float> Acoords;

// arrays for HETATMS

std::vector<std::string> Hatom_name;
std::vector<std::string> Hresidue;
std::vector<float> Hcoords;
public:
std::vector<float> Acoords;
Environment(std::string pdb);
void append_name(std::string name);
void append_Hname(std::string name);
void append_residue(std::string res);
void append_Hresidue(std::string res);
void append_coords(std::string x, std::string y, std::string z);
void append_Hcoords(std::string x, std::string y, std::string z);


};
#endif