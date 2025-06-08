#version 330 core

// Regular vertex attributes
layout (location = 0) in vec3 vertex_coordinates;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

// Instance attributes
layout (location = 3) in vec3 instance_position;
layout (location = 4) in vec3 instance_color;
layout (location = 5) in float instance_scale;
layout (location = 6) in float instance_rotation;

// Uniforms
uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform mat4 normal_matrix;

// Outputs to fragment shader
out vec3 fragment_position;
out vec3 fragment_normal;
out vec3 base_color;

void main()
{
    // Create rotation matrix for Y-axis rotation
    float cosRot = cos(instance_rotation);
    float sinRot = sin(instance_rotation);
    mat3 rotationMatrix = mat3(
        cosRot,  0.0, sinRot,
        0.0,     1.0, 0.0,
        -sinRot, 0.0, cosRot
    );
    
    // Apply instance transformations
    vec3 rotatedPosition = rotationMatrix * vertex_coordinates;
    vec3 scaledPosition = rotatedPosition * instance_scale;
    vec3 worldPosition = scaledPosition + instance_position;
    
    // Transform to view space
    vec4 viewPosition = model_view_matrix * vec4(worldPosition, 1.0);
    fragment_position = viewPosition.xyz;
    
    // Transform normal (also apply rotation)
    vec3 rotatedNormal = rotationMatrix * vertex_normal;
    fragment_normal = normalize((normal_matrix * vec4(rotatedNormal, 0.0)).xyz);
    
    // Combine vertex color with instance color
    // Since we set vertex colors to white, this effectively uses instance color
    base_color = vertex_color * instance_color;
    
    // Final position
    gl_Position = projection_matrix * viewPosition;
}