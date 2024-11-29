#include "my_drawing/svg_generator.h"

void plot_line_chart(
        const std::string &file_path,
        const std::vector<std::vector<double>> &x_values_list,
        const std::vector<std::vector<double>> &y_values_list,
        const std::string &title,
        const std::string &x_label,
        const std::string &y_label,
        const std::vector<std::string> &legend_labels
) {
    /* Create SVG renderer */
    std::string svg_string;
    CSVG_Renderer renderer(canvas_width, canvas_height, svg_string);

    /* Render line chart */
    render_line_chart(renderer, x_values_list, y_values_list, title, x_label, y_label, legend_labels);

    /* Save SVG to file */
    std::ofstream svg_file(file_path);
    svg_file << svg_string;
    svg_file.close();
}

void render_line_chart(
        CSVG_Renderer& renderer,
        const std::vector<std::vector<double>> &x_values_list,
        const std::vector<std::vector<double>> &y_values_list,
        const std::string &title,
        const std::string &x_label,
        const std::string &y_label,
        const std::vector<std::string> &legend_labels
) {
    /* Determine bounds for all lines */
    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::lowest();
    double y_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::lowest();

    /* Find min and max values for all lines and take the global extremes */
    for (size_t i = 0; i < x_values_list.size(); ++i) {
        x_min = std::min(x_min, *std::min_element(x_values_list[i].begin(), x_values_list[i].end()));
        x_max = std::max(x_max, *std::max_element(x_values_list[i].begin(), x_values_list[i].end()));
        y_min = std::min(y_min, *std::min_element(y_values_list[i].begin(), y_values_list[i].end()));
        y_max = std::max(y_max, *std::max_element(y_values_list[i].begin(), y_values_list[i].end()));
    }

    /* Scaling factors */
    double x_scale = plot_width / (x_max - x_min);
    double y_scale = plot_height / (y_max - y_min);

    /* Begin rendering */
    renderer.Begin_Render();

    /* Background rectangle for the plot */
    drawing::Rectangle background(padding, padding, plot_width, plot_height);
    background.Set_Fill_Color(RGBColor::From_UInt32(white));
    background.Set_Stroke_Color(RGBColor::From_UInt32(black));
    renderer.Render_Rectangle(background);

    /* Draw grid lines and ticks */
    for (size_t i = 0; i <= num_ticks; ++i) {
        /* Vertical grid lines and X ticks */
        double x = padding + (static_cast<double>(i) / double(num_ticks)) * plot_width;
        drawing::Line grid_line_x(x, padding, x, padding + plot_height);
        grid_line_x.Set_Stroke_Color(RGBColor::From_UInt32(gray));
        grid_line_x.Set_Stroke_Width(0.5);
        renderer.Render_Line(grid_line_x);

        /* X tick labels */
        double x_tick_value = x_min + (static_cast<double>(i) / double(num_ticks)) * (x_max - x_min);
        std::ostringstream tick_label_x;
        tick_label_x << x_tick_value;
        drawing::Text x_tick_label(x, canvas_height - padding + 20, tick_label_x.str());
        x_tick_label.Set_Anchor(drawing::Text::TextAnchor::MIDDLE);
        x_tick_label.Set_Transform("rotate(-12.5, " + std::to_string(x) + ", " + std::to_string(canvas_height - padding + 20) + ")");
        renderer.Render_Text(x_tick_label);

        /* Horizontal grid lines and Y ticks */
        double y = padding + plot_height - (static_cast<double>(i) / double(num_ticks)) * plot_height;
        drawing::Line grid_line_y(padding, y, padding + plot_width, y);
        grid_line_y.Set_Stroke_Color(RGBColor::From_UInt32(gray));
        grid_line_y.Set_Stroke_Width(0.5);
        renderer.Render_Line(grid_line_y);

        /* Y tick labels */
        double y_tick_value = y_min + (static_cast<double>(i) / double(num_ticks)) * (y_max - y_min);
        std::ostringstream tick_label_y;
        tick_label_y << y_tick_value;
        drawing::Text y_tick_label(padding - 20, y, tick_label_y.str());
        y_tick_label.Set_Anchor(drawing::Text::TextAnchor::END);
        renderer.Render_Text(y_tick_label);
    }

    /* Prepare colors for each line */
    std::vector<uint32_t> colors = {red, green, blue, magenta, cyan, yellow};

    /* Plot each line */
    for (size_t line_index = 0; line_index < x_values_list.size(); ++line_index) {
        const auto& x_values = x_values_list[line_index];
        const auto& y_values = y_values_list[line_index];

        /* Scaling */
        std::vector<std::pair<double, double>> scaled_points;
        for (size_t i = 0; i < x_values.size(); ++i) {
            double x = padding + (x_values[i] - x_min) * x_scale;
            double y = padding + plot_height - (y_values[i] - y_min) * y_scale;
            scaled_points.emplace_back(x, y);
        }

        /* Create polyline and add each point */
        drawing::PolyLine polyline(scaled_points[0].first, scaled_points[0].second);
        scaled_points.erase(scaled_points.begin());  /* Throw away the first point, it is already added */
        for (auto& point : scaled_points)
            polyline.Add_Point(point.first, point.second);

        /* Choose the color for the current line and render it */
        uint32_t color = colors[line_index % colors.size()];
        polyline.Set_Stroke_Color(RGBColor::From_UInt32(color));
        polyline.Set_Fill_Opacity(0.0);
        polyline.Set_Stroke_Width(2.0);
        polyline.Set_Stroke_Opacity(0.5);
        renderer.Render_PolyLine(polyline);
    }

    /* Draw axis labels */
    drawing::Text x_axis_label(canvas_width / 2., canvas_height - padding / 4., x_label);
    x_axis_label.Set_Anchor(drawing::Text::TextAnchor::MIDDLE);
    renderer.Render_Text(x_axis_label);

    drawing::Text y_axis_label(20, canvas_height / 2. - padding / 4., y_label);
    y_axis_label.Set_Anchor(drawing::Text::TextAnchor::MIDDLE);
    y_axis_label.Set_Transform("rotate(-90, 20, " + std::to_string(canvas_height / 2) + ")");
    renderer.Render_Text(y_axis_label);

    /* Draw title */
    drawing::Text chart_title(canvas_width / 2., padding - padding / 4., title);
    chart_title.Set_Anchor(drawing::Text::TextAnchor::MIDDLE);
    chart_title.Set_Font_Weight(drawing::Text::FontWeight::BOLD);
    renderer.Render_Text(chart_title);

    /* Draw legend */
    if (!legend_labels.empty()) {
        double legend_x = canvas_width - padding - 150;
        double legend_y = padding + 20;
        /* Draw legend box and labels */
        for (size_t i = 0; i < legend_labels.size(); ++i) {
            /* Draw legend box */
            drawing::Rectangle legend_box(legend_x, legend_y - 10, 10, 10);
            legend_box.Set_Stroke_Color(RGBColor::From_UInt32(colors[i % colors.size()]));
            legend_box.Set_Fill_Color(RGBColor::From_UInt32(colors[i % colors.size()]));
            renderer.Render_Rectangle(legend_box);

            /* Draw legend label */
            drawing::Text legend_label(legend_x + 15, legend_y, legend_labels[i]);
            legend_label.Set_Anchor(drawing::Text::TextAnchor::START);
            renderer.Render_Text(legend_label);

            legend_y += 20;
        }
    }

    /* Finalize rendering */
    renderer.Finalize_Render();
}