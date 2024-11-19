#include "dataloader/dataloader.h"

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
        x = std::strtod(line_ptr, &line_ptr);
        line_ptr++; /* Skip comma */

        y = std::strtod(line_ptr, &line_ptr);
        line_ptr++; /* Skip comma */

        z = std::strtod(line_ptr, &line_ptr);

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
    const size_t file_size = ftell(in_fp);
    fseek(in_fp, 0, SEEK_SET);

    /* Read the file into a buffer into memory (RAM) */
    char *buffer = new char[file_size];
    fread(buffer, 1, file_size, in_fp);
    fclose(in_fp);

    /* Parse per lines */
    std::vector<std::string_view> lines;
    size_t start_index = 0;  /* Start of the line */
    for (size_t i = 0; i < file_size; i++) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + start_index, i - start_index);
            start_index = i + 1;  /* Skip the newline */
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

    const auto max_num_threads = static_cast<size_t>(omp_get_max_threads());
    const size_t chunk_size = num_lines / max_num_threads;

    #pragma omp parallel for default(none) shared(lines, data, num_lines, max_num_threads, chunk_size)
    for (size_t i = 0; i < max_num_threads; i++) {
        const size_t start = i * chunk_size;
        /* Final thread may have to handle a little more elements */
        const size_t end = (i == max_num_threads - 1) ? num_lines : (i + 1) * chunk_size;

        char line[max_byte_value];
        /* Parse the lines */
        for (size_t j = start; j < end; j++) {
            strncpy(line, lines[j].data(), lines[j].size());
            line[lines[j].size()] = '\0';  /* Null-terminate for strtok */

            /* Split the line manually */
            strtok(line, ",");  /* Skip first token -- datetime */

            /*
             * Parse x, y, z values directly
             * When using strtod or std::strtod here, the parallel version gets progressively
             * slower with the number of threads used -- I have no idea why, but atof saved the day
             */
            char *token = strtok(nullptr, ",");
            data.x[j] = atof(token);

            token = strtok(nullptr, ",");
            data.y[j] = atof(token);

            token = strtok(nullptr, ",");
            data.z[j] = atof(token);
        }
    }

    /* Clean up */
    delete[] buffer;
}
