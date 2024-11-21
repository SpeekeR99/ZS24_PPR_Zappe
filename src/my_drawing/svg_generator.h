#pragma once

#include <algorithm>
#include <fstream>

#include "drawing/SVGRenderer.h"
#include "utils/utils.h"

/**
 * Uses the SVGRenderer.h from https://github.com/SmartCGMS/common.git
 * Plots a line chart to a svg file
 * @param file_path File path to save the chart
 * @param x_values_list List of x values for each line
 * @param y_values_list List of y values for each line
 * @param title Title of the chart
 * @param x_label X-axis label
 * @param y_label Y-axis label
 * @param legend_labels Labels for each line
 */
void plot_line_chart(
    const std::string &file_path,
    const std::vector<std::vector<double>> &x_values_list,
    const std::vector<std::vector<double>> &y_values_list,
    const std::string &title = "Line Chart",
    const std::string &x_label = "X-axis",
    const std::string &y_label = "Y-axis",
    const std::vector<std::string> &legend_labels = {}
);

/**
 * Uses the SVGRenderer.h from https://github.com/SmartCGMS/common.git
 * Renders a line chart using the given SVG renderer
 * @param renderer SVG renderer
 * @param x_values_list List of x values for each line
 * @param y_values_list List of y values for each line
 * @param title Title of the chart
 * @param x_label X-axis label
 * @param y_label Y-axis label
 * @param legend_labels Labels for each line
 */
void render_line_chart(
    CSVG_Renderer& renderer,
    const std::vector<std::vector<double>> &x_values_list,
    const std::vector<std::vector<double>> &y_values_list,
    const std::string &title = "Line Chart",
    const std::string &x_label = "X-axis",
    const std::string &y_label = "Y-axis",
    const std::vector<std::string> &legend_labels = {}
);
