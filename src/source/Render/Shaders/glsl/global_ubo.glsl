// Global matrices and scene environment UBO (binding point 0)
layout(std140) uniform GlobalMatrices {
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_Model;
    mat4 u_MVP;      // Proj * View * Model
    vec4 u_Time;     // x=WorldTime, y=DeltaTime, zw=reserved
};
