####################################################################################
## This configuration emulates the lumi pattern of LB 1040 in run 363830 (fill 7314).
## Any of the filled bunch crossings can be chosen as the central
## bunch crossing, so the timing of the out-of-time pile-up will vary
## significantly from event to event.  Cavern Background is
## independent of the bunch pattern. (Compatible with 25ns cavern
## background.)
####################################################################################

from Digitization.DigitizationFlags import digitizationFlags

digitizationFlags.cavernIgnoresBeamInt = True

digitizationFlags.bunchSpacing = 25 # This now sets the bunch slot length.

digitizationFlags.BeamIntensityPattern = [0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.46, 1.51, 1.55, 1.58, 1.45, 1.48, 1.54, 1.54, 1.60, 1.56, 1.55, 1.58, 1.55, 1.51, 1.45, 1.48, 1.36, 1.34, 1.29, 1.33, 1.46, 1.46, 1.40, 1.42, 1.39, 1.36, 1.29, 1.34, 1.27, 1.26, 1.22, 1.27, 1.33, 1.36, 1.31, 1.37, 1.32, 1.33, 1.29, 1.34, 1.24, 1.24, 1.19, 1.26, 1.31, 1.31, 1.24, 1.14, 0., 0., 0., 0., 0., 0., 0., 1.16, 1.33, 1.37, 1.39, 1.30, 1.34, 1.41, 1.43, 1.50, 1.47, 1.43, 1.47, 1.42, 1.38, 1.33, 1.34, 1.23, 1.20, 1.17, 1.20, 1.29, 1.29, 1.23, 1.26, 1.26, 1.23, 1.18, 1.20, 1.14, 1.12, 1.08, 1.13, 1.15, 1.16, 1.12, 1.17, 1.12, 1.12, 1.09, 1.13, 1.05, 1.07, 1.03, 1.08, 1.09, 1.11, 1.06, 0.96, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.42, 1.51, 1.58, 1.60, 1.51, 1.47, 1.50, 1.50, 1.51, 1.49, 1.45, 1.48, 1.48, 1.45, 1.38, 1.40, 1.32, 1.29, 1.24, 1.27, 1.31, 1.34, 1.28, 1.31, 1.28, 1.29, 1.22, 1.25, 1.19, 1.19, 1.13, 1.18, 1.18, 1.21, 1.16, 1.21, 1.17, 1.19, 1.14, 1.17, 1.12, 1.13, 1.06, 1.11, 1.11, 1.13, 1.04, 0.97, 0., 0., 0., 0., 0., 0., 0., 0.90, 1.08, 1.11, 1.15, 1.09, 1.14, 1.19, 1.20, 1.23, 1.20, 1.17, 1.19, 1.13, 1.10, 1.06, 1.08, 0.99, 0.97, 0.93, 0.95, 1.00, 0.99, 0.94, 0.97, 0.95, 0.95, 0.90, 0.92, 0.86, 0.86, 0.82, 0.86, 0.86, 0.87, 0.85, 0.88, 0.87, 0.89, 0.85, 0.88, 0.83, 0.83, 0.79, 0.83, 0.84, 0.84, 0.78, 0.73, 0., 0., 0., 0., 0., 0., 0., 0.68, 0.87, 0.90, 0.94, 0.90, 0.96, 1.00, 1.01, 1.03, 1.02, 0.99, 1.01, 0.96, 0.94, 0.91, 0.91, 0.84, 0.82, 0.80, 0.81, 0.84, 0.84, 0.80, 0.82, 0.81, 0.80, 0.76, 0.77, 0.73, 0.71, 0.68, 0.70, 0.72, 0.72, 0.71, 0.73, 0.73, 0.73, 0.71, 0.73, 0.69, 0.70, 0.67, 0.70, 0.72, 0.74, 0.70, 0.70, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.34, 1.47, 1.50, 1.55, 1.45, 1.41, 1.46, 1.46, 1.46, 1.43, 1.40, 1.43, 1.39, 1.33, 1.31, 1.33, 1.26, 1.22, 1.18, 1.21, 1.24, 1.24, 1.18, 1.21, 1.18, 1.18, 1.12, 1.15, 1.09, 1.08, 1.02, 1.07, 1.08, 1.08, 1.03, 1.08, 1.05, 1.06, 1.01, 1.05, 0.97, 0.98, 0.95, 0.97, 0.97, 1.00, 0.91, 0.83, 0., 0., 0., 0., 0., 0., 0., 0.81, 0.99, 1.02, 1.06, 1.04, 1.08, 1.12, 1.13, 1.13, 1.12, 1.08, 1.09, 1.01, 1.00, 0.96, 0.96, 0.91, 0.89, 0.85, 0.86, 0.89, 0.89, 0.84, 0.84, 0.84, 0.82, 0.78, 0.81, 0.77, 0.76, 0.73, 0.75, 0.75, 0.76, 0.73, 0.76, 0.75, 0.76, 0.73, 0.75, 0.71, 0.72, 0.69, 0.71, 0.70, 0.71, 0.65, 0.61, 0., 0., 0., 0., 0., 0., 0., 0.60, 0.79, 0.82, 0.86, 0.85, 0.89, 0.91, 0.93, 0.95, 0.93, 0.91, 0.92, 0.87, 0.86, 0.84, 0.84, 0.79, 0.77, 0.75, 0.76, 0.78, 0.76, 0.74, 0.75, 0.75, 0.73, 0.71, 0.71, 0.68, 0.67, 0.65, 0.66, 0.67, 0.67, 0.66, 0.68, 0.66, 0.66, 0.66, 0.68, 0.66, 0.67, 0.64, 0.66, 0.67, 0.68, 0.67, 0.64, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.36, 1.52, 1.56, 1.61, 1.46, 1.46, 1.47, 1.47, 1.51, 1.48, 1.43, 1.47, 1.44, 1.42, 1.36, 1.37, 1.31, 1.27, 1.21, 1.23, 1.31, 1.31, 1.24, 1.27, 1.23, 1.22, 1.15, 1.17, 1.11, 1.10, 1.05, 1.08, 1.08, 1.10, 1.07, 1.11, 1.10, 1.11, 1.04, 1.09, 1.03, 1.02, 0.96, 1.00, 1.02, 1.02, 0.94, 0.87, 0., 0., 0., 0., 0., 0., 0., 0.79, 0.99, 1.01, 1.05, 1.01, 1.05, 1.08, 1.10, 1.13, 1.12, 1.07, 1.10, 1.03, 1.00, 0.98, 0.97, 0.91, 0.88, 0.85, 0.86, 0.90, 0.90, 0.86, 0.87, 0.84, 0.84, 0.80, 0.81, 0.76, 0.76, 0.73, 0.75, 0.77, 0.77, 0.75, 0.78, 0.78, 0.77, 0.74, 0.77, 0.73, 0.72, 0.70, 0.73, 0.73, 0.74, 0.71, 0.65, 0., 0., 0., 0., 0., 0., 0., 0.60, 0.79, 0.83, 0.87, 0.86, 0.93, 0.95, 0.95, 0.95, 0.95, 0.92, 0.93, 0.86, 0.85, 0.83, 0.83, 0.79, 0.77, 0.75, 0.75, 0.76, 0.76, 0.72, 0.74, 0.75, 0.74, 0.70, 0.70, 0.68, 0.67, 0.65, 0.66, 0.66, 0.67, 0.65, 0.67, 0.65, 0.66, 0.64, 0.66, 0.64, 0.66, 0.64, 0.66, 0.66, 0.66, 0.65, 0.65, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.42, 1.54, 1.61, 1.65, 1.51, 1.45, 1.51, 1.50, 1.58, 1.55, 1.49, 1.53, 1.58, 1.54, 1.47, 1.48, 1.39, 1.36, 1.30, 1.33, 1.45, 1.45, 1.37, 1.40, 1.32, 1.33, 1.26, 1.30, 1.16, 1.18, 1.14, 1.20, 1.19, 1.24, 1.19, 1.25, 1.20, 1.21, 1.18, 1.20, 1.08, 1.10, 1.07, 1.10, 1.11, 1.13, 1.06, 0.98, 0., 0., 0., 0., 0., 0., 0., 0.89, 1.09, 1.11, 1.17, 1.13, 1.10, 1.13, 1.13, 1.14, 1.14, 1.10, 1.12, 1.15, 1.13, 1.09, 1.10, 1.03, 1.01, 0.98, 0.98, 1.02, 1.03, 0.97, 1.00, 0.97, 0.96, 0.91, 0.92, 0.87, 0.86, 0.82, 0.85, 0.86, 0.87, 0.84, 0.88, 0.86, 0.86, 0.82, 0.86, 0.83, 0.84, 0.81, 0.84, 0.85, 0.86, 0.82, 0.75, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.33, 1.46, 1.48, 1.50, 1.39, 1.37, 1.37, 1.39, 1.44, 1.44, 1.37, 1.43, 1.48, 1.49, 1.41, 1.44, 1.36, 1.32, 1.24, 1.30, 1.40, 1.40, 1.31, 1.36, 1.35, 1.36, 1.26, 1.30, 1.22, 1.22, 1.15, 1.20, 1.26, 1.28, 1.20, 1.25, 1.21, 1.22, 1.16, 1.19, 1.12, 1.14, 1.06, 1.10, 1.14, 1.15, 1.06, 0.99, 0., 0., 0., 0., 0., 0., 0., 0.84, 1.06, 1.10, 1.11, 1.09, 1.05, 1.09, 1.09, 1.11, 1.10, 1.06, 1.07, 1.07, 1.05, 1.01, 1.01, 0.97, 0.93, 0.89, 0.90, 0.95, 0.93, 0.88, 0.90, 0.90, 0.88, 0.84, 0.85, 0.81, 0.79, 0.75, 0.78, 0.79, 0.80, 0.77, 0.80, 0.79, 0.80, 0.77, 0.79, 0.76, 0.76, 0.71, 0.75, 0.75, 0.76, 0.70, 0.66, 0., 0., 0., 0., 0., 0., 0., 0.60, 0.77, 0.78, 0.83, 0.82, 0.81, 0.82, 0.84, 0.80, 0.81, 0.79, 0.81, 0.81, 0.81, 0.78, 0.79, 0.75, 0.74, 0.72, 0.73, 0.73, 0.73, 0.71, 0.72, 0.69, 0.69, 0.68, 0.68, 0.66, 0.65, 0.63, 0.65, 0.64, 0.65, 0.65, 0.67, 0.64, 0.65, 0.64, 0.67, 0.64, 0.65, 0.64, 0.66, 0.66, 0.67, 0.64, 0.63, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.28, 1.40, 1.40, 1.44, 1.37, 1.32, 1.33, 1.36, 1.37, 1.36, 1.26, 1.32, 1.37, 1.35, 1.25, 1.28, 1.25, 1.20, 1.13, 1.17, 1.26, 1.25, 1.15, 1.18, 1.17, 1.15, 1.07, 1.11, 1.04, 1.07, 0.99, 1.04, 1.01, 1.07, 0.99, 1.04, 1.00, 1.04, 0.95, 0.97, 0.91, 0.91, 0.85, 0.89, 0.87, 0.86, 0.79, 0.74, 0., 0., 0., 0., 0., 0., 0., 0.83, 1.02, 1.04, 1.07, 1.06, 1.01, 1.04, 1.06, 1.09, 1.09, 1.05, 1.08, 1.08, 1.06, 1.03, 1.03, 0.96, 0.94, 0.91, 0.92, 0.96, 0.96, 0.92, 0.94, 0.91, 0.90, 0.85, 0.86, 0.82, 0.81, 0.78, 0.79, 0.80, 0.82, 0.80, 0.83, 0.81, 0.83, 0.81, 0.84, 0.78, 0.78, 0.75, 0.78, 0.79, 0.80, 0.75, 0.70, 0., 0., 0., 0., 0., 0., 0., 0.63, 0.79, 0.82, 0.86, 0.85, 0.83, 0.85, 0.87, 0.87, 0.87, 0.84, 0.86, 0.88, 0.87, 0.83, 0.84, 0.82, 0.79, 0.76, 0.78, 0.79, 0.79, 0.74, 0.76, 0.76, 0.76, 0.71, 0.73, 0.71, 0.70, 0.67, 0.68, 0.68, 0.69, 0.67, 0.69, 0.67, 0.68, 0.66, 0.69, 0.66, 0.67, 0.65, 0.68, 0.68, 0.69, 0.67, 0.65, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.29, 1.42, 1.46, 1.48, 1.37, 1.35, 1.36, 1.37, 1.41, 1.40, 1.34, 1.37, 1.38, 1.38, 1.30, 1.33, 1.25, 1.23, 1.15, 1.21, 1.29, 1.28, 1.21, 1.25, 1.21, 1.20, 1.11, 1.15, 1.08, 1.08, 1.00, 1.07, 1.10, 1.13, 1.06, 1.11, 1.07, 1.09, 1.03, 1.06, 1.00, 1.01, 0.96, 0.99, 1.01, 1.03, 0.94, 0.87, 0., 0., 0., 0., 0., 0., 0., 0.85, 1.01, 1.03, 1.07, 1.04, 1.02, 1.04, 1.05, 1.07, 1.05, 1.02, 1.03, 1.06, 1.04, 1.00, 1.00, 0.98, 0.94, 0.90, 0.91, 0.95, 0.94, 0.89, 0.90, 0.90, 0.89, 0.84, 0.85, 0.82, 0.81, 0.78, 0.80, 0.82, 0.83, 0.79, 0.82, 0.82, 0.81, 0.78, 0.81, 0.77, 0.77, 0.74, 0.77, 0.78, 0.79, 0.75, 0.70, 0., 0., 0., 0., 0., 0., 0., 0.63, 0.80, 0.82, 0.87, 0.86, 0.84, 0.85, 0.86, 0.89, 0.88, 0.85, 0.88, 0.86, 0.86, 0.83, 0.83, 0.79, 0.78, 0.74, 0.75, 0.78, 0.78, 0.75, 0.76, 0.75, 0.75, 0.71, 0.72, 0.69, 0.68, 0.65, 0.67, 0.68, 0.69, 0.67, 0.70, 0.69, 0.69, 0.66, 0.69, 0.66, 0.67, 0.65, 0.68, 0.69, 0.70, 0.67, 0.66, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.32, 1.48, 1.49, 1.53, 1.42, 1.37, 1.39, 1.40, 1.46, 1.42, 1.36, 1.40, 1.46, 1.42, 1.36, 1.38, 1.29, 1.25, 1.18, 1.22, 1.32, 1.31, 1.23, 1.28, 1.28, 1.27, 1.18, 1.22, 1.13, 1.13, 1.07, 1.11, 1.15, 1.17, 1.10, 1.15, 1.15, 1.15, 1.09, 1.12, 1.06, 1.05, 0.99, 1.02, 1.02, 1.04, 0.98, 0.90, 0., 0., 0., 0., 0., 0., 0., 0.86, 1.01, 1.03, 1.06, 1.07, 1.03, 1.05, 1.06, 1.06, 1.06, 1.03, 1.04, 1.05, 1.05, 0.99, 1.01, 0.97, 0.94, 0.89, 0.92, 0.95, 0.94, 0.88, 0.90, 0.89, 0.88, 0.83, 0.85, 0.82, 0.81, 0.77, 0.79, 0.79, 0.80, 0.77, 0.80, 0.77, 0.80, 0.77, 0.79, 0.77, 0.77, 0.73, 0.76, 0.75, 0.76, 0.70, 0.65, 0., 0., 0., 0., 0., 0., 0., 0.62, 0.78, 0.80, 0.85, 0.83, 0.80, 0.83, 0.85, 0.86, 0.86, 0.84, 0.86, 0.88, 0.88, 0.85, 0.85, 0.81, 0.79, 0.75, 0.77, 0.80, 0.80, 0.76, 0.77, 0.78, 0.77, 0.73, 0.73, 0.71, 0.70, 0.67, 0.68, 0.69, 0.70, 0.68, 0.71, 0.70, 0.71, 0.68, 0.71, 0.68, 0.68, 0.67, 0.69, 0.70, 0.72, 0.69, 0.70, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.32, 1.44, 1.48, 1.47, 1.39, 1.37, 1.37, 1.37, 1.43, 1.41, 1.34, 1.39, 1.42, 1.40, 1.35, 1.36, 1.28, 1.24, 1.19, 1.22, 1.32, 1.31, 1.22, 1.27, 1.33, 1.33, 1.24, 1.27, 1.18, 1.19, 1.13, 1.17, 1.20, 1.23, 1.18, 1.23, 1.17, 1.19, 1.12, 1.17, 1.08, 1.08, 1.01, 1.07, 1.09, 1.10, 1.01, 0.93, 0., 0., 0., 0., 0., 0., 0., 0.86, 1.04, 1.05, 1.08, 1.05, 1.03, 1.05, 1.08, 1.09, 1.08, 1.04, 1.07, 1.12, 1.08, 1.05, 1.07, 1.01, 0.98, 0.94, 0.96, 0.98, 0.99, 0.94, 0.96, 0.93, 0.94, 0.88, 0.90, 0.85, 0.84, 0.80, 0.83, 0.82, 0.83, 0.81, 0.85, 0.82, 0.83, 0.79, 0.83, 0.79, 0.80, 0.76, 0.79, 0.80, 0.82, 0.77, 0.70, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.35, 1.46, 1.51, 1.53, 1.43, 1.39, 1.43, 1.43, 1.49, 1.46, 1.40, 1.44, 1.44, 1.40, 1.33, 1.35, 1.30, 1.26, 1.19, 1.23, 1.35, 1.33, 1.25, 1.29, 1.28, 1.28, 1.18, 1.21, 1.14, 1.15, 1.10, 1.14, 1.17, 1.19, 1.14, 1.19, 1.16, 1.16, 1.09, 1.14, 1.07, 1.07, 1.00, 1.04, 1.08, 1.09, 1.00, 0.93, 0., 0., 0., 0., 0., 0., 0., 0.85, 1.02, 1.02, 1.06, 1.05, 1.03, 1.04, 1.05, 1.08, 1.07, 1.04, 1.07, 1.10, 1.09, 1.03, 1.05, 1.00, 0.98, 0.93, 0.95, 0.99, 0.98, 0.92, 0.95, 0.91, 0.92, 0.86, 0.88, 0.83, 0.83, 0.78, 0.82, 0.83, 0.84, 0.81, 0.84, 0.83, 0.84, 0.80, 0.82, 0.78, 0.79, 0.75, 0.78, 0.78, 0.80, 0.74, 0.70, 0., 0., 0., 0., 0., 0., 0., 0.65, 0.80, 0.82, 0.85, 0.85, 0.84, 0.84, 0.87, 0.90, 0.89, 0.85, 0.87, 0.88, 0.87, 0.83, 0.84, 0.80, 0.79, 0.75, 0.76, 0.80, 0.80, 0.76, 0.78, 0.78, 0.78, 0.74, 0.75, 0.72, 0.72, 0.69, 0.70, 0.72, 0.73, 0.71, 0.73, 0.71, 0.72, 0.70, 0.72, 0.70, 0.71, 0.68, 0.70, 0.72, 0.74, 0.71, 0.69, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.33, 1.45, 1.47, 1.50, 1.40, 1.37, 1.39, 1.39, 1.43, 1.42, 1.38, 1.40, 1.47, 1.46, 1.39, 1.41, 1.32, 1.28, 1.24, 1.27, 1.36, 1.37, 1.28, 1.32, 1.26, 1.28, 1.17, 1.22, 1.15, 1.15, 1.08, 1.12, 1.14, 1.16, 1.12, 1.16, 1.15, 1.18, 1.11, 1.15, 1.07, 1.07, 1.03, 1.06, 1.07, 1.08, 1.00, 0.93, 0., 0., 0., 0., 0., 0., 0., 0.84, 1.01, 1.00, 1.04, 1.01, 1.02, 1.02, 1.03, 1.02, 1.03, 0.99, 1.01, 0.99, 1.00, 0.95, 0.96, 0.91, 0.90, 0.86, 0.88, 0.90, 0.90, 0.84, 0.86, 0.84, 0.85, 0.81, 0.83, 0.79, 0.78, 0.74, 0.77, 0.76, 0.78, 0.75, 0.78, 0.73, 0.75, 0.72, 0.75, 0.71, 0.72, 0.67, 0.70, 0.68, 0.71, 0.63, 0.60, 0., 0., 0., 0., 0., 0., 0., 0.63, 0.78, 0.82, 0.85, 0.84, 0.81, 0.85, 0.87, 0.89, 0.89, 0.87, 0.89, 0.88, 0.87, 0.84, 0.85, 0.82, 0.80, 0.76, 0.78, 0.81, 0.80, 0.76, 0.78, 0.76, 0.76, 0.72, 0.73, 0.70, 0.70, 0.67, 0.69, 0.70, 0.71, 0.70, 0.72, 0.69, 0.70, 0.68, 0.71, 0.68, 0.70, 0.67, 0.70, 0.72, 0.73, 0.70, 0.66, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.32, 1.45, 1.49, 1.50, 1.39, 1.36, 1.38, 1.39, 1.45, 1.43, 1.38, 1.42, 1.44, 1.44, 1.34, 1.37, 1.28, 1.26, 1.18, 1.22, 1.34, 1.33, 1.24, 1.28, 1.27, 1.27, 1.18, 1.22, 1.13, 1.13, 1.04, 1.12, 1.14, 1.17, 1.10, 1.17, 1.17, 1.19, 1.12, 1.15, 1.07, 1.09, 1.02, 1.06, 1.08, 1.11, 1.01, 0.94, 0., 0., 0., 0., 0., 0., 0., 0.86, 1.01, 1.03, 1.06, 1.03, 1.02, 1.03, 1.04, 1.03, 1.04, 0.99, 1.01, 1.00, 1.02, 0.96, 0.98, 0.92, 0.92, 0.87, 0.88, 0.90, 0.92, 0.87, 0.88, 0.89, 0.88, 0.85, 0.86, 0.82, 0.82, 0.78, 0.80, 0.81, 0.82, 0.78, 0.81, 0.79, 0.81, 0.78, 0.82, 0.76, 0.78, 0.73, 0.76, 0.75, 0.78, 0.71, 0.67, 0., 0., 0., 0., 0., 0., 0., 0.64, 0.80, 0.82, 0.86, 0.86, 0.84, 0.85, 0.86, 0.86, 0.88, 0.83, 0.85, 0.85, 0.87, 0.82, 0.84, 0.79, 0.79, 0.76, 0.77, 0.77, 0.79, 0.75, 0.77, 0.77, 0.76, 0.74, 0.75, 0.72, 0.71, 0.68, 0.70, 0.70, 0.72, 0.70, 0.72, 0.70, 0.71, 0.69, 0.73, 0.67, 0.69, 0.67, 0.71, 0.71, 0.73, 0.69, 0.69, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.31, 1.44, 1.45, 1.48, 1.40, 1.38, 1.37, 1.40, 1.42, 1.43, 1.36, 1.39, 1.41, 1.41, 1.33, 1.36, 1.28, 1.27, 1.19, 1.23, 1.33, 1.33, 1.25, 1.27, 1.26, 1.27, 1.21, 1.24, 1.14, 1.16, 1.10, 1.14, 1.18, 1.22, 1.16, 1.21, 1.17, 1.21, 1.13, 1.17, 1.10, 1.10, 1.05, 1.08, 1.10, 1.12, 1.04, 0.95, 0., 0., 0., 0., 0., 0., 0., 0.89, 1.07, 1.08, 1.11, 1.08, 1.06, 1.08, 1.09, 1.11, 1.11, 1.06, 1.09, 1.08, 1.07, 1.02, 1.03, 0.97, 0.95, 0.92, 0.94, 0.96, 0.96, 0.92, 0.94, 0.92, 0.92, 0.87, 0.89, 0.85, 0.84, 0.80, 0.82, 0.82, 0.84, 0.81, 0.84, 0.83, 0.83, 0.80, 0.83, 0.79, 0.79, 0.76, 0.78, 0.78, 0.80, 0.74, 0.69, 0., 0., 0., 0., 0., 0., 0., 0.65, 0.80, 0.81, 0.86, 0.85, 0.85, 0.85, 0.87, 0.88, 0.87, 0.86, 0.87, 0.87, 0.87, 0.83, 0.84, 0.82, 0.79, 0.77, 0.77, 0.78, 0.79, 0.76, 0.77, 0.77, 0.77, 0.74, 0.75, 0.71, 0.71, 0.69, 0.71, 0.71, 0.72, 0.70, 0.72, 0.72, 0.72, 0.71, 0.73, 0.71, 0.72, 0.70, 0.72, 0.74, 0.74, 0.71, 0.72, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.44, 1.57, 1.54, 1.59, 1.51, 1.46, 1.45, 1.46, 1.56, 1.54, 1.43, 1.48, 1.54, 1.51, 1.40, 1.43, 1.39, 1.33, 1.26, 1.31, 1.41, 1.41, 1.31, 1.36, 1.37, 1.35, 1.26, 1.30, 1.25, 1.24, 1.15, 1.22, 1.28, 1.30, 1.21, 1.28, 1.28, 1.27, 1.17, 1.23, 1.15, 1.17, 1.08, 1.14, 1.15, 1.18, 1.06, 0.99, 0., 0., 0., 0., 0., 0., 0., 0.93, 1.08, 1.08, 1.11, 1.09, 1.07, 1.08, 1.09, 1.13, 1.13, 1.07, 1.10, 1.15, 1.14, 1.07, 1.08, 1.05, 1.01, 0.95, 0.97, 1.04, 1.03, 0.95, 0.97, 0.97, 0.96, 0.89, 0.91, 0.87, 0.86, 0.82, 0.84, 0.85, 0.86, 0.82, 0.86, 0.87, 0.87, 0.83, 0.86, 0.84, 0.85, 0.79, 0.83, 0.85, 0.86, 0.80, 0.72, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.34, 1.44, 1.47, 1.46, 1.43, 1.40, 1.39, 1.39, 1.46, 1.46, 1.35, 1.40, 1.52, 1.53, 1.42, 1.46, 1.41, 1.38, 1.27, 1.32, 1.44, 1.45, 1.36, 1.39, 1.32, 1.32, 1.24, 1.28, 1.22, 1.22, 1.15, 1.21, 1.24, 1.26, 1.19, 1.25, 1.23, 1.24, 1.17, 1.20, 1.16, 1.17, 1.09, 1.08, 1.12, 1.15, 1.06, 0.96, 0., 0., 0., 0., 0., 0., 0., 0.89, 1.04, 1.05, 1.08, 1.06, 1.04, 1.05, 1.06, 1.12, 1.12, 1.07, 1.09, 1.07, 1.07, 1.01, 1.03, 0.97, 0.95, 0.91, 0.93, 0.97, 0.98, 0.93, 0.95, 0.91, 0.90, 0.84, 0.87, 0.81, 0.81, 0.77, 0.80, 0.81, 0.83, 0.80, 0.83, 0.88, 0.89, 0.85, 0.87, 0.84, 0.84, 0.78, 0.78, 0.80, 0.82, 0.77, 0.72, 0., 0., 0., 0., 0., 0., 0., 0.66, 0.80, 0.83, 0.85, 0.86, 0.84, 0.85, 0.86, 0.90, 0.90, 0.87, 0.89, 0.86, 0.85, 0.82, 0.83, 0.79, 0.78, 0.75, 0.76, 0.80, 0.79, 0.76, 0.78, 0.79, 0.78, 0.75, 0.75, 0.73, 0.71, 0.68, 0.71, 0.73, 0.74, 0.73, 0.74, 0.73, 0.74, 0.72, 0.73, 0.71, 0.71, 0.69, 0.71, 0.74, 0.75, 0.71, 0.70, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.34, 1.47, 1.50, 1.52, 1.45, 1.40, 1.42, 1.44, 1.48, 1.46, 1.40, 1.44, 1.45, 1.43, 1.32, 1.37, 1.33, 1.30, 1.23, 1.25, 1.38, 1.37, 1.29, 1.31, 1.27, 1.27, 1.18, 1.22, 1.16, 1.18, 1.12, 1.16, 1.18, 1.20, 1.15, 1.20, 1.19, 1.22, 1.14, 1.17, 1.11, 1.11, 1.07, 1.07, 1.07, 1.09, 1.01, 0.93, 0., 0., 0., 0., 0., 0., 0., 0.88, 1.02, 1.04, 1.07, 1.06, 1.03, 1.04, 1.05, 1.09, 1.08, 1.04, 1.06, 1.07, 1.05, 1.01, 1.03, 0.99, 0.96, 0.91, 0.93, 0.98, 0.97, 0.92, 0.94, 0.91, 0.90, 0.84, 0.87, 0.82, 0.82, 0.78, 0.82, 0.82, 0.82, 0.79, 0.84, 0.87, 0.87, 0.84, 0.86, 0.82, 0.82, 0.78, 0.78, 0.79, 0.80, 0.74, 0.71, 0., 0., 0., 0., 0., 0., 0., 0.65, 0.78, 0.81, 0.84, 0.83, 0.82, 0.82, 0.85, 0.87, 0.86, 0.84, 0.87, 0.88, 0.88, 0.84, 0.85, 0.82, 0.80, 0.76, 0.78, 0.81, 0.81, 0.76, 0.79, 0.80, 0.80, 0.75, 0.77, 0.74, 0.73, 0.69, 0.71, 0.73, 0.74, 0.72, 0.73, 0.74, 0.75, 0.74, 0.75, 0.71, 0.73, 0.70, 0.73, 0.75, 0.75, 0.71, 0.70, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.32, 1.45, 1.46, 1.50, 1.41, 1.38, 1.37, 1.39, 1.49, 1.47, 1.38, 1.42, 1.46, 1.45, 1.34, 1.38, 1.32, 1.28, 1.20, 1.25, 1.37, 1.39, 1.28, 1.33, 1.30, 1.33, 1.23, 1.27, 1.20, 1.21, 1.14, 1.20, 1.26, 1.26, 1.19, 1.24, 1.24, 1.26, 1.17, 1.21, 1.13, 1.16, 1.08, 1.08, 1.12, 1.15, 1.03, 0.95, 0., 0., 0., 0., 0., 0., 0., 0.87, 1.02, 1.01, 1.05, 1.03, 1.01, 1.02, 1.03, 1.08, 1.08, 1.03, 1.06, 1.11, 1.10, 1.03, 1.04, 0.99, 0.96, 0.92, 0.93, 0.99, 1.00, 0.94, 0.95, 0.92, 0.92, 0.86, 0.88, 0.85, 0.84, 0.79, 0.81, 0.83, 0.85, 0.83, 0.85, 0.87, 0.89, 0.86, 0.87, 0.83, 0.83, 0.78, 0.78, 0.81, 0.82, 0.77, 0.72, 0., 0., 0., 0., 0., 0., 0., 0.66, 0.79, 0.81, 0.85, 0.84, 0.81, 0.83, 0.85, 0.88, 0.88, 0.85, 0.87, 0.91, 0.89, 0.86, 0.87, 0.84, 0.82, 0.78, 0.80, 0.84, 0.83, 0.79, 0.80, 0.79, 0.79, 0.73, 0.74, 0.72, 0.71, 0.67, 0.69, 0.72, 0.71, 0.70, 0.72, 0.76, 0.76, 0.74, 0.75, 0.73, 0.73, 0.70, 0.73, 0.75, 0.77, 0.71, 0.70, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.30, 1.45, 1.47, 1.49, 1.38, 1.37, 1.39, 1.40, 1.44, 1.43, 1.37, 1.41, 1.41, 1.41, 1.31, 1.34, 1.29, 1.26, 1.18, 1.22, 1.32, 1.35, 1.25, 1.29, 1.23, 1.26, 1.15, 1.21, 1.12, 1.14, 1.04, 1.11, 1.14, 1.18, 1.11, 1.16, 1.17, 1.20, 1.11, 1.16, 1.09, 1.10, 0.99, 1.03, 1.05, 1.06, 0.98, 0.91, 0., 0., 0., 0., 0., 0., 0., 0.86, 0.99, 1.02, 1.06, 1.01, 1.00, 1.02, 1.03, 1.08, 1.07, 1.04, 1.04, 1.07, 1.06, 1.02, 1.03, 0.98, 0.96, 0.92, 0.93, 1.00, 0.99, 0.93, 0.95, 0.93, 0.92, 0.86, 0.87, 0.84, 0.83, 0.79, 0.81, 0.84, 0.86, 0.82, 0.85, 0.86, 0.87, 0.84, 0.86, 0.81, 0.83, 0.78, 0.77, 0.78, 0.80, 0.76, 0.70, 0., 0., 0., 0., 0., 0., 0., 0.66, 0.79, 0.82, 0.85, 0.83, 0.82, 0.83, 0.84, 0.88, 0.88, 0.85, 0.87, 0.86, 0.86, 0.82, 0.84, 0.79, 0.78, 0.75, 0.77, 0.81, 0.81, 0.77, 0.78, 0.80, 0.79, 0.75, 0.76, 0.73, 0.72, 0.69, 0.71, 0.74, 0.75, 0.73, 0.75, 0.74, 0.76, 0.73, 0.75, 0.72, 0.72, 0.69, 0.72, 0.75, 0.77, 0.72, 0.73, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.]

from AthenaCommon.BeamFlags import jobproperties
jobproperties.Beam.bunchSpacing = 25

