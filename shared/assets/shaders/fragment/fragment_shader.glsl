#version 330

in vec3 fragment_position;
in vec3 fragment_normal;
in vec3 base_color;

out vec4 fragment_color;

void main()
{
    // Light properties
    vec3 light_position = vec3(20.0, 50.0, 20.0);  // Position in view space
    vec3 light_color = vec3(1.0, 1.0, 0.9);        // Slightly warm white light
    float light_intensity = 1.0;
    
    // Material properties
    vec3 ambient_color = base_color * 0.2;         // Ambient component
    vec3 diffuse_color = base_color * 0.8;         // Diffuse component
    vec3 specular_color = vec3(0.5);               // Specular component
    float shininess = 32.0;                        // Shininess factor for specular
    
    // Calculate lighting vectors
    vec3 normal = normalize(fragment_normal);
    vec3 light_dir = normalize(light_position - fragment_position);
    vec3 view_dir = normalize(-fragment_position);  // Camera is at origin in view space
    vec3 reflect_dir = reflect(-light_dir, normal);
    
    // Calculate ambient component
    vec3 ambient = ambient_color;
    
    // Calculate diffuse component
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * diffuse_color * light_color * light_intensity;
    
    // Calculate specular component
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = spec * specular_color * light_color * light_intensity;
    
    // Calculate final color
    vec3 result = ambient + diffuse + specular;
    
    // Output final color
    fragment_color = vec4(result, 1.0);
}