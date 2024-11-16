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
    while (fgets(buffer, sizeof(buffer), in_fp)) {
        char *line_ptr = buffer;

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

void load_data_parallel(const std::string &filepath, patient_data &data) {
    /* Open the file (in binary mode!) */
    FILE *in_fp = fopen(filepath.c_str(), "rb");
    if (!in_fp) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Get the file size */
    fseek(in_fp, 0, SEEK_END);
    size_t fileSize = ftell(in_fp);
    fseek(in_fp, 0, SEEK_SET);

    /* Read the file into a buffer into memory (RAM) */
    char *buffer = new char[fileSize];
    fread(buffer, 1, fileSize, in_fp);
    fclose(in_fp);

    /* Parse per lines */
    std::vector<std::string_view> lines;
    size_t startIdx = 0;  /* Start of the line */
    for (size_t i = 0; i < fileSize; i++) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + startIdx, i - startIdx);
            startIdx = i + 1;  /* Skip the newline */
        }
    }

    /* Skip the header */
    lines.erase(lines.begin());

    /* Clean the data */
    data.x.clear();
    data.y.clear();
    data.z.clear();

    /* Resize the vectors to the number of lines -- big advantage here over the previous load */
    size_t num_lines = lines.size();
    data.x.resize(num_lines);
    data.y.resize(num_lines);
    data.z.resize(num_lines);

    auto max_num_threads = static_cast<size_t>(omp_get_max_threads());
    size_t chunk_size = num_lines / max_num_threads;

//    #pragma omp parallel for
    for (size_t i = 0; i < max_num_threads; i++) {
        size_t start = i * chunk_size;
        /* Final thread may have to handle a little more elements */
        size_t end = (i == max_num_threads - 1) ? num_lines : (i + 1) * chunk_size;

        /* Parse the lines */
        for (size_t j = start; j < end; j++) {
            char *line_ptr = const_cast<char*>(lines[j].data());

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

            /* Store the values */
            data.x[j] = x;
            data.y[j] = y;
            data.z[j] = z;
        }
    }

    /* Clean up */
    delete[] buffer;
}
