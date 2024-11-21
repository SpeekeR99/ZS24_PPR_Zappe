#pragma once

#include <algorithm>
#include <fstream>

#include "drawing/SVGRenderer.h"
#include "utils/utils.h"

/** Padding for the plot */
constexpr size_t padding = 50;
/** Canvas width */
constexpr size_t canvas_width = 800.0;
/** Canvas height */
constexpr size_t canvas_height = 600.0;
/** Plot area width */
constexpr size_t plot_width = canvas_width - 2 * padding;
/** Plot area height */
constexpr size_t plot_height = canvas_height - 2 * padding;
/** Number of ticks on each axis */
constexpr size_t num_ticks = 10;

/** White color */
constexpr size_t white = 0xffffff;
/** Gray color (grid) */
constexpr size_t gray = 0xc8c8c8;
/** Black color */
constexpr size_t black = 0x000000;
/** Red color */
constexpr size_t red = 0xff0000;
/** Green color */
constexpr size_t green = 0x00ff00;
/** Blue color */
constexpr size_t blue = 0x0000ff;
/** Magenta color */
constexpr size_t magenta = 0xff00ff;
/** Yellow color */
constexpr size_t yellow = 0xffff00;
/** Cyan color */
constexpr size_t cyan = 0x00ffff;

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
