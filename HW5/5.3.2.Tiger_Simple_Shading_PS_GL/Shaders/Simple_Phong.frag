#version 330

uniform struct LightInfo {
  vec4 Position;   // Light position in eye coords
  vec3 La;         // Ambient light intesity
  vec3 L;          // Direct light intensity
  vec3 Spot_direction; // Spot direction in eye coords
  float Spot_cutoff_angle; // in degrees in [0, 90]
  float Spot_exponent;
  vec3 Attenuation_factors;
} Light;

uniform struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
} Material;

uniform bool use_halfway_vector;
uniform bool apply_spot_light;
uniform bool apply_attenuation;

uniform sampler2D u_base_texture;
uniform bool u_flag_texture_mapping;
uniform float u_alpha;

uniform bool u_flag_reflection;
uniform sampler2D u_reflection_texture;
uniform vec2 u_window_size;

uniform bool u_flag_shadow;
uniform sampler2D u_shadow_map;
uniform mat4 u_shadow_matrix;
uniform float u_caster_alpha;

in vec3 Position_EC;
in vec3 Normal_EC;
in vec2 TexCoord;

layout(location = 0) out vec4 FragColor;

vec3 PhongShading(vec3 position, vec3 n, float shadow_factor) {
    vec3 ambient_color = Material.Ka;
    vec3 diffuse_color = Material.Kd;
    if (u_flag_texture_mapping) {
        vec3 tex_color = texture(u_base_texture, TexCoord).rgb;
        ambient_color = tex_color;
        diffuse_color = tex_color;
    }
    vec3 color_sum = Light.La * ambient_color;
    vec3 s = normalize(Light.Position.xyz - position);

    float attenuation = 1.0;
    if (apply_attenuation) {
        float d = length(Light.Position.xyz - position);
        attenuation = 1.0 / (Light.Attenuation_factors.x + Light.Attenuation_factors.y * d
            + Light.Attenuation_factors.z * d * d);
    }

    float local_scale_factor = 1.0;
    if (apply_spot_light) {  
	    float Spot_cutoff_angle = clamp(Light.Spot_cutoff_angle, 0.0, 90.0);
	    vec3 spot_dir = normalize(Light.Spot_direction);

	    float tmp_float = dot(-s, spot_dir);
	    if (tmp_float >= cos(radians(Spot_cutoff_angle))) {
		    local_scale_factor = pow(tmp_float, Light.Spot_exponent);
	    }
	    else {
		    local_scale_factor = 0.0;
        }
    }
    if (local_scale_factor > 0.0) {
        vec3 diffuse = vec3(0.0);
        vec3 spec = vec3(0.0);

        float sDotN = max(dot(s, n), 0.0);
        if(sDotN > 0.0) {
            diffuse = diffuse_color * sDotN;
            vec3 v = normalize(-position.xyz);
            if (use_halfway_vector) {
                vec3 h = normalize(s + v);
                spec = Material.Ks *
                        pow(max(dot(n, h), 0.0), Material.Shininess);
            }
            else {
                vec3 r = reflect(-s, n);
                spec = Material.Ks *
                     pow(max(dot(r, v), 0.0), Material.Shininess);
            }
        }
        color_sum += Light.L * (diffuse + spec) * local_scale_factor * attenuation * shadow_factor;
    }
    return color_sum;
}

float compute_shadow() {
    vec4 light_space_pos = u_shadow_matrix * vec4(Position_EC, 1.0);
    vec3 proj = light_space_pos.xyz / light_space_pos.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0)
        return 1.0;
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 1.0;

    float bias = 0.0015;
    float current_depth = proj.z;
    float lit = 0.0;
    vec2 texel_size = 1.0 / vec2(textureSize(u_shadow_map, 0));
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closest_depth = texture(u_shadow_map, proj.xy + vec2(x, y) * texel_size).r;
            lit += (current_depth - bias > closest_depth) ? 0.0 : 1.0;
        }
    }
    return lit / 9.0;
}

void main() {
    if (u_caster_alpha < 1.0) {
        float dither = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
        if (dither > u_caster_alpha)
            discard;
    }

    float shadow_factor = 1.0;
    if (u_flag_shadow)
        shadow_factor = compute_shadow();
    vec3 shaded_color = PhongShading(Position_EC, normalize(Normal_EC), shadow_factor);
    if (u_flag_reflection) {
        vec2 reflection_uv = gl_FragCoord.xy / u_window_size;
        vec4 reflection_color = texture(u_reflection_texture, reflection_uv);
        shaded_color = mix(shaded_color, reflection_color.rgb, 0.5 * reflection_color.a);
    }
    FragColor = vec4(shaded_color, u_alpha);
}