#include "dataloader/dataloader.h"

void load_data(const std::string &filepath, patient_data &data) {
    /* Open the file */
    std::ifstream in_fp(filepath);
    if (!in_fp) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Clean the data */
    data.x.clear();
    data.y.clear();
    data.z.clear();

    /* Skip header */
    std::string line;
    std::getline(in_fp, line);

    /* Read the data */
    std::string datetime;
    decimal x, y, z;

    while (std::getline(in_fp, line)) {
        std::stringstream ss(line);
        std::getline(ss, datetime, ',');
        ss >> x;
        ss.ignore(1, ',');
        ss >> y;
        ss.ignore(1, ',');
        ss >> z;

        data.x.push_back(x);
        data.y.push_back(y);
        data.z.push_back(z);
    }

    /* Close the file */
    in_fp.close();
}

void load_data_fast(const std::string& filepath, patient_data &data) {
    /* Open the file */
    FILE *in_fp = fopen(filepath.c_str(), "r");
    if (!in_fp) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Clean the data */
    data.x.clear();
    data.y.clear();
    data.z.clear();

    /* Skip header */
    char header[max_byte_value];
    fgets(header, sizeof(header), in_fp);

    /* Read the data */
    char datetime[max_byte_value];
    decimal x, y, z;
    #ifndef _USE_FLOAT
    while(fscanf(in_fp, "%[^,],%lf,%lf,%lf\n", datetime, &x, &y, &z) != EOF) {
        data.x.push_back(x);
        data.y.push_back(y);
        data.z.push_back(z);
    }
    #else
    while(fscanf(in_fp, "%[^,],%f,%f,%f\n", datetime, &x, &y, &z) != EOF) {
        data.x.push_back(x);
        data.y.push_back(y);
        data.z.push_back(z);
    }
    #endif

    /* Close the file */
    fclose(in_fp);
}

void load_data_super_fast(const std::string& filepath, patient_data &data) {
    /* Open the file */
    FILE *in_fp = fopen(filepath.c_str(), "r");
    if (!in_fp) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Clean the data */
    data.x.clear();
    data.y.clear();
    data.z.clear();

    /* Start with 1 MB as expected size -- will be changed later in the parsing loop */
    data.x.resize(MB);
    data.y.resize(MB);
    data.z.resize(MB);

    /* Set a large buffer size */
    setvbuf(in_fp, nullptr, _IOFBF, MB);

    /* Skip header */
    char buffer[max_byte_value];
    fgets(buffer, sizeof(buffer), in_fp);

    size_t index = 0;

    /* Read the data in large chunks */
    decimal x, y, z;
    while (fgets(buffer, sizeof(buffer), in_fp)) {
        char *line_ptr = buffer;

        /* Find the end of the datetime and move to the numeric data */
        while (*line_ptr && *line_ptr != ',')
            line_ptr++;
        line_ptr++; /* Skip comma */

        /* Parse x, y, z values directly */
        #ifndef _USE_FLOAT
        x = std::strtod(line_ptr, &line_ptr);
        line_ptr++; /* Skip comma */

        y = std::strtod(line_ptr, &line_ptr);
        line_ptr++; /* Skip comma */

        z = std::strtod(line_ptr, &line_ptr);
        #else
        x = std::strtof(line_ptr, &line_ptr);
        line_ptr++; /* Skip comma */

        y = std::strtof(line_ptr, &line_ptr);
        line_ptr++; /* Skip comma */

        z = std::strtof(line_ptr, &line_ptr);
        #endif

        /* Ensure we have enough space */
        if (index >= data.x.size()) {
            /* Increase size exponentially to minimize reallocations */
            auto new_size = static_cast<size_t>(2 * data.x.size());
            data.x.resize(new_size);
            data.y.resize(new_size);
            data.z.resize(new_size);
        }

        /* Store values */
        data.x[index] = x;
        data.y[index] = y;
        data.z[index] = z;
        index++;
    }

    /* Resize vectors to the actual number of elements */
    data.x.resize(index);
    data.y.resize(index);
    data.z.resize(index);

    /* Clean up */
    fclose(in_fp);
}
