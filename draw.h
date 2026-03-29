#pragma once

#include "json.hpp"

/**
 * @brief Generate and save a Koch snowflake image.
 * @param params JSON object containing KochParams fields.
 */
void genKoch      (const nlohmann::json & params);

/**
 * @brief Generate and save a Mandelbrot set image.
 * @param params JSON object containing MandelParams fields.
 */
void genMandel    (const nlohmann::json & params);

/**
 * @brief Generate and save a Sierpinski triangle image.
 * @param params JSON object containing SierParams fields.
 */
void genSierpinski(const nlohmann::json & params);

/**
 * @brief Generate and save a fractal tree image.
 * @param params JSON object containing TreeParams fields.
 */
void genTree      (const nlohmann::json & params);
