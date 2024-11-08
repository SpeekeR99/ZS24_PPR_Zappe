#include "dataloader.h"

void load_data(const std::string& filepath, patient_data &data) {
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
    double x, y, z;
    while(fscanf(in_fp, "%[^,],%lf,%lf,%lf\n", datetime, &x, &y, &z) != EOF) {
        data.x.push_back(x);
        data.y.push_back(y);
        data.z.push_back(z);
    }

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
    data.x.resize(MB1);
    data.y.resize(MB1);
    data.z.resize(MB1);

    /* Set a large buffer size */
    setvbuf(in_fp, nullptr, _IOFBF, KB64); /* Set a 64 KB buffer for faster reading */

    /* Skip header */
    char header[max_byte_value];
    fgets(header, sizeof(header), in_fp);

    /* Prepare a 1 MB buffer for reading data */
    char *buffer = new char[MB1];
    size_t index = 0;

    /* Read the data in large chunks */
    while (fgets(buffer, MB1, in_fp)) {
        char *line_ptr = buffer;
        while (*line_ptr) {
            /* Find the end of the datetime and move to the numeric data */
            while (*line_ptr && *line_ptr != ',')
                line_ptr++;
            line_ptr++; /* Skip comma */

            /* Parse x, y, z values directly */
            double x = std::strtod(line_ptr, &line_ptr);
            line_ptr++; /* Skip comma */

            double y = std::strtod(line_ptr, &line_ptr);
            line_ptr++; /* Skip comma */

            double z = std::strtod(line_ptr, &line_ptr);
            line_ptr++; /* Skip newline */

            /* Ensure we have enough space */
            if (index >= data.x.size()) {
                /* Increase size exponentially to minimize reallocations */
                auto new_size = static_cast<size_t>(1.5 * data.x.size());
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
    }

    /* Resize vectors to the actual number of elements */
    data.x.resize(index);
    data.y.resize(index);
    data.z.resize(index);

    /* Clean up */
    delete[] buffer;
    fclose(in_fp);
}
