#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "Env.h"
#include "parm.h"
#include "Sim.h"
#include <math.h>
#include <thread>
#include <iomanip>

simulation::simulation(Environment& temp1, parm& temp2, float step){

    coord = std::make_unique<Environment>(temp1);
    top = std::make_unique<parm>(temp2);



    velocities = std::vector<float>(coord->Acoords.size(), 0);
    forces = std::vector<float>(coord->Acoords.size(), 0);
    
    }


void simulation::update_coord(float step_size, int frames){
    int counting = 0;
    for(int i = 0; i < frames; i++){
        VerletAlg(step_size);
        
        if(i % 20 == 0) {
            std::cout << i << std::endl;
            exports(counting);
            counting += 1;

        }
    }
}

void simulation::force_additions(){ //all forces on protien for bond/FF interactions

    //bond forces
    std::unordered_map<std::string, std::vector<T> >::iterator bwh = top->values.find("BONDS_WITHOUT_HYDROGEN");
        std::vector<T>& BWoutH = bwh->second;
    
    std::unordered_map<std::string, std::vector<T> >::iterator bih = top->values.find("BONDS_INC_HYDROGEN");
        std::vector<T>& BIH = bih->second;

    std::unordered_map<std::string, std::vector<T> >::iterator bfc = top->values.find("BOND_FORCE_CONSTANT");
        std::vector<T>& BForceC = bfc->second;

    std::unordered_map<std::string, std::vector<T> >::iterator bev = top->values.find("BOND_EQUIL_VALUE");
        std::vector<T>& BEQV = bev->second;
    for(int i = 0; i < BWoutH.size() - 1; i+=3){ //BWoutH.size()
        spring_force( std::get<int>(BWoutH[i]) / 3,std::get<int>(BWoutH[i + 1]) / 3, std::get<float>(BForceC[std::get<int>(BWoutH[i + 2]) - 1]), std::get<float>(BEQV[std::get<int>(BWoutH[i + 2]) - 1]) );

    }
    for(int i = 0; i < BIH.size(); i+=3){ // this is a bond which include hydrogen. I want to stiffen interaction so as to reduce computational complexity. I can set up a method for selecting a model and integrate it into there
        spring_force( std::get<int>(BIH[i]) / 3,std::get<int>(BIH[i + 1]) / 3, std::get<float>(BForceC[std::get<int>(BIH[i + 2]) - 1]), std::get<float>(BEQV[std::get<int>(BIH[i + 2]) - 1]) );
    }


    //angle forces
    std::unordered_map<std::string, std::vector<T> >::iterator awh = top->values.find("ANGLES_WITHOUT_HYDROGEN");
        std::vector<T>& AWoutH = awh->second;

    std::unordered_map<std::string, std::vector<T> >::iterator aih = top->values.find("ANGLES_INC_HYDROGEN");
        std::vector<T>& AIH = aih->second;

    std::unordered_map<std::string, std::vector<T> >::iterator afc = top->values.find("ANGLE_FORCE_CONSTANT");
        std::vector<T>& AForceC = afc->second;

    std::unordered_map<std::string, std::vector<T> >::iterator aev = top->values.find("ANGLE_EQUIL_VALUE");
        std::vector<T>& AEQV = aev->second;
    for(int i = 0; i < AWoutH.size() - 1; i+=4){ //BWoutH.size()
        angle_force( std::get<int>(AWoutH[i]) / 4,std::get<int>(AWoutH[i + 1]) / 4, std::get<int>(AWoutH[i + 2]) / 4, std::get<float>(AForceC[std::get<int>(AWoutH[i + 3]) - 1]), std::get<float>(AEQV[std::get<int>(AWoutH[i + 3]) - 1]));

    }

    
}

void simulation::angle_force(int atom1, int atom2, int atom3, float k, float eq){
    std::vector<float> dispab;
    std::vector<float> dunit_vect;
    float theta; 
    theta_from_dot(atom1, atom2, atom3, theta);
    float angforce = 0.5 * (k*(theta - eq));
    forces[atom1 * 3]

}

void simulation::theta_from_dot(int& atom1, int& atom2, int& atom3, float& theta){
    float dotac = 0;
    dot(atom1, atom2, dotac);
    float magba;
    magnitude(atom1, atom2, magba);
    float magbc;
    magnitude(atom2, atom3, magbc);
    theta = acos(dotac/(magba*magbc));


}

void simulation::dot(int& atom1, int& atom2, float& val){
    for(int i = 0; i < 3; i++){
        val += coord->Acoords[atom1 * 3 + i] * coord->Acoords[atom2 * 3 + i];
    }
}

void simulation::spring_force(int atom1, int atom2, float k, float eq){
    std::vector<float> disp;
    float dmag;
    std::vector<float> dunit_vect;

    displacement_vect(disp, atom1, atom2);
    magnitude(disp, dmag);
    unit_vector(dmag, disp, dunit_vect);

    for(int i = 0; i < disp.size(); i ++){
        float force = -0.5 * (-k * (disp[i] - eq * dunit_vect[i]));
        forces[atom2 * 3 + i] += force;
        forces[atom1 * 3 + i] -= force;

    }

}

void simulation::unit_vector(float& mag, std::vector<float> d, std::vector<float>& unitv){
    for(int i = 0; i < d.size(); i++){
        unitv.push_back(d[i]/mag);
    }
}

void simulation::magnitude(std::vector<float>& object, float& mag){
    float temp = 0;
    for(int i = 0; i < object.size(); i++){
        temp+= object[i]*object[i];
    }
    temp = std::sqrt(temp);
    mag = temp;
}

void simulation::displacement_vect(std::vector<float>& d, int atom1, int atom2){
    for(int i = 0; i < 3; i++){
        d.push_back(coord->Acoords[atom1 * 3 + i] - coord->Acoords[atom2 * 3 + i]);
    }
}

void simulation::VerletAlg(float& step_size){
    std::unordered_map<std::string, std::vector<T> >::iterator ms = top->values.find("MASS");
        std::vector<T>& Mass = ms->second;
for(int atom = 0; atom < velocities.size(); atom++){
    velocities[atom] = velocities[atom] + (forces[atom] * step_size/(2 * std::get<float>(Mass[atom/(int)3])));
}
for(int atom = 0; atom < coord->Acoords.size(); atom++){
    coord->Acoords[atom] = coord->Acoords[atom] + velocities[atom]*step_size;
}
forces.assign(forces.size(), 0);
force_additions();
for(int atom = 0; atom < velocities.size(); atom++){
   velocities[atom] = velocities[atom] + forces[atom]*step_size / (2 * std::get<float>(Mass[atom/(int)3]) );//6.02214086e+26
}


}

void simulation::exports(int count){
    std::fstream temp_file;
    temp_file.open("coord_data/temp.crd", std::ios::out);
    std::unordered_map<std::string, std::vector<T> >::iterator nm = top->values.find("ATOM_NAME");
        std::vector<T>& name = nm->second;
    temp_file << std::left << std::setw(20) << "title" << "\n";
    std::cout << count << std::endl;
    for(int i = 0; i < coord->Acoords.size() - 1; i+=3){
        temp_file << "ATOM "
        << std::right << std::setw(5) << i/3 + 1 
        << std::left << std::setw(4) << std::get<std::string>(name[i/3])
        << "    "
        << std::right << std::setw(3) << i/20 + 1
        << "     "
        << std::fixed << std::setprecision(3)
        << std::setw(8) << (coord->Acoords[i]) << std::setw(8) << (coord->Acoords[i+1]) << std::setw(8) << (coord->Acoords[i+2]) << "\n";
    }
}



