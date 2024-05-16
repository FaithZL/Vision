//
// Created by Zero on 10/12/2022.
//

#pragma once

#include "math/basic_types.h"

namespace vision {

struct MeasuredSS {
    const char *name{};
    float3 sigma_s;
    float3 sigma_a;
};

static MeasuredSS SubsurfaceParameterTable[] = {
    // From "A Practical Model for Subsurface Light Transport"
    // Jensen, Marschner, Levoy, Hanrahan
    // Proc SIGGRAPH 2001
    {"Apple", float3(2.29, 2.39, 1.97), float3(0.0030, 0.0034, 0.046)},
    {"Chicken1", float3(0.15, 0.21, 0.38), float3(0.015, 0.077, 0.19)},
    {"Chicken2", float3(0.19, 0.25, 0.32), float3(0.018, 0.088, 0.20)},
    {"Cream", float3(7.38, 5.47, 3.15), float3(0.0002, 0.0028, 0.0163)},
    {"Ketchup", float3(0.18, 0.07, 0.03), float3(0.061, 0.97, 1.45)},
    {"Marble", float3(2.19, 2.62, 3.00), float3(0.0021, 0.0041, 0.0071)},
    {"Potato", float3(0.68, 0.70, 0.55), float3(0.0024, 0.0090, 0.12)},
    {"Skimmilk", float3(0.70, 1.22, 1.90), float3(0.0014, 0.0025, 0.0142)},
    {"Skin1", float3(0.74, 0.88, 1.01), float3(0.032, 0.17, 0.48)},
    {"Skin2", float3(1.09, 1.59, 1.79), float3(0.013, 0.070, 0.145)},
    {"Spectralon", float3(11.6, 20.4, 14.9), float3(0.00, 0.00, 0.00)},
    {"Wholemilk", float3(2.55, 3.21, 3.77), float3(0.0011, 0.0024, 0.014)},
    // From "Acquiring Scattering Properties of Participating Media by
    // Dilution",
    // Narasimhan, Gupta, Donner, Ramamoorthi, Nayar, Jensen
    // Proc SIGGRAPH 2006
    {"Lowfat Milk", float3(0.89187, 1.5136, 2.532), float3(0.002875, 0.00575, 0.0115)},
    {"Reduced Milk", float3(2.4858, 3.1669, 4.5214), float3(0.0025556, 0.0051111, 0.012778)},
    {"Regular Milk", float3(4.5513, 5.8294, 7.136), float3(0.0015333, 0.0046, 0.019933)},
    {"Espresso", float3(0.72378, 0.84557, 1.0247), float3(4.7984, 6.5751, 8.8493)},
    {"Mint Mocha Coffee", float3(0.31602, 0.38538, 0.48131), float3(3.772, 5.8228, 7.82)},
    {"Lowfat Soy Milk", float3(0.30576, 0.34233, 0.61664), float3(0.0014375, 0.0071875, 0.035937)},
    {"Regular Soy Milk", float3(0.59223, 0.73866, 1.4693), float3(0.0019167, 0.0095833, 0.065167)},
    {"Lowfat Chocolate Milk", float3(0.64925, 0.83916, 1.1057), float3(0.0115, 0.0368, 0.1564)},
    {"Regular Chocolate Milk", float3(1.4585, 2.1289, 2.9527), float3(0.010063, 0.043125, 0.14375)},
    {"Coke", float3(8.9053e-05, 8.372e-05, 0), float3(0.10014, 0.16503, 0.2468)},
    {"Pepsi", float3(6.1697e-05, 4.2564e-05, 0), float3(0.091641, 0.14158, 0.20729)},
    {"Sprite", float3(6.0306e-06, 6.4139e-06, 6.5504e-06), float3(0.001886, 0.0018308, 0.0020025)},
    {"Gatorade", float3(0.0024574, 0.003007, 0.0037325), float3(0.024794, 0.019289, 0.008878)},
    {"Chardonnay", float3(1.7982e-05, 1.3758e-05, 1.2023e-05), float3(0.010782, 0.011855, 0.023997)},
    {"White Zinfandel", float3(1.7501e-05, 1.9069e-05, 1.288e-05), float3(0.012072, 0.016184, 0.019843)},
    {"Merlot", float3(2.1129e-05, 0, 0), float3(0.11632, 0.25191, 0.29434)},
    {"Budweiser Beer", float3(2.4356e-05, 2.4079e-05, 1.0564e-05), float3(0.011492, 0.024911, 0.057786)},
    {"Coors Light Beer", float3(5.0922e-05, 4.301e-05, 0), float3(0.006164, 0.013984, 0.034983)},
    {"Clorox", float3(0.0024035, 0.0031373, 0.003991), float3(0.0033542, 0.014892, 0.026297)},
    {"Apple Juice", float3(0.00013612, 0.00015836, 0.000227), float3(0.012957, 0.023741, 0.052184)},
    {"Cranberry Juice", float3(0.00010402, 0.00011646, 7.8139e-05), float3(0.039437, 0.094223, 0.12426)},
    {"Grape Juice", float3(5.382e-05, 0, 0), float3(0.10404, 0.23958, 0.29325)},
    {"Ruby Grapefruit Juice", float3(0.011002, 0.010927, 0.011036), float3(0.085867, 0.18314, 0.25262)},
    {"White Grapefruit Juice", float3(0.22826, 0.23998, 0.32748), float3(0.0138, 0.018831, 0.056781)},
    {"Shampoo", float3(0.0007176, 0.0008303, 0.0009016), float3(0.014107, 0.045693, 0.061717)},
    {"Strawberry Shampoo", float3(0.00015671, 0.00015947, 1.518e-05), float3(0.01449, 0.05796, 0.075823)},
    {"Head & Shoulders Shampoo", float3(0.023805, 0.028804, 0.034306), float3(0.084621, 0.15688, 0.20365)},
    {"Lemon Tea Powder", float3(0.040224, 0.045264, 0.051081), float3(2.4288, 4.5757, 7.2127)},
    {"Orange Powder", float3(0.00015617, 0.00017482, 0.0001762), float3(0.001449, 0.003441, 0.007863)},
    {"Pink Lemonade Powder", float3(0.00012103, 0.00013073, 0.00012528), float3(0.001165, 0.002366, 0.003195)},
    {"Cappuccino Powder", float3(1.8436, 2.5851, 2.1662), float3(35.844, 49.547, 61.084)},
    {"Salt Powder", float3(0.027333, 0.032451, 0.031979), float3(0.28415, 0.3257, 0.34148)},
    {"Sugar Powder", float3(0.00022272, 0.00025513, 0.000271), float3(0.012638, 0.031051, 0.050124)},
    {"Suisse Mocha Powder", float3(2.7979, 3.5452, 4.3365), float3(17.502, 27.004, 35.433)},
    {"Pacific Ocean Surface Water", float3(0.0001764, 0.00032095, 0.00019617), float3(0.031845, 0.031324, 0.030147)}};
}// namespace vision