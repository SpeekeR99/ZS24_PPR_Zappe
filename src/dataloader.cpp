#include "dataloader.h"

void load_data(std::string filepath, patient_data &data) {
    /* Open the file */
    std::ifstream in_fp(filepath);
    if (!in_fp.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Clear the data */
    data.x.clear();
    data.x.reserve(1000000);
    data.y.clear();
    data.y.reserve(1000000);
    data.z.clear();
    data.z.reserve(1000000);

    /* Read the data */
    std::string line;
    std::getline(in_fp, line); /* Skip the header */
    std::string datetime;
    double x, y, z;
    while (std::getline(in_fp, line)) {
        std::istringstream ss(line);
        ss >> datetime >> x >> y >> z;
        data.x.emplace_back(x);
        data.y.emplace_back(y);
        data.z.emplace_back(z);
    }

    /* Close the file */
    in_fp.close();
}

