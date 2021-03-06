#ifndef __PBR_H__
#define __PBR_H__

/******************************************************************************
 * diffuse BRDF:
 * 
 *  Based on UE4 implementation
 *    (https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)
 * 
 *  which is based on Disney
 *    (https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf)
 * 
 *  Attributes:
 *    base color, metallic, roughness, (cavity)
 * 
 *  Rendering Equation:
 *    integrateOmega(f(l, v) * Li(l) * (n dot l), dwi)
 * 
 *    where
 * 
 *    f(l, v) = diffuse + specular BRDF
 *    Li(l) = amount of incident light
 *    (n dot l) = cosine factor
 *  
 *  Lambertian diffuse for f(l, v):
 *    diffuseColor / pi
 * 
 *  Cook-Torrance microfacet specular for f(l, v) :
 *    D(h) * F(v, h) * G(l, v, h) / 4 * (n dot l) * (n dot v)
 * 
 *    where
 * 
 *    D: Concentration/area of microgeometry that could reflect from l to v, function of roughness
 *    (Disney GGX/Trowbridge-Reitz NDF)
 *      D(h) = pow(alpha, 2) / (pi * pow((pow(n dot h, 2) * (pow(alpha, 2) - 1) + 1), 2))
 *      with
 *        alpha = pow(roughness, 2)
 * 
 *    G: % of surface points with m = h that are not shadowed or masked, function of roughness
 *    (Schlick with k = alpha / 2)
 *      G(l, v, h) = G_1(l) * G_1(v)
 *      G_1(v) = n dot v / ((n dot v) * (1 - k) + k)
 *      with
 *        k = pow(roughness + 1, 2) / 8
 * 
 *    F: Fresnel reflectance of active surface points for l, m = h, function of metallicness
 *    (Schlick Fresnel with spherical gaussian approx to replace power)
 *      F(v, h) = F0 + (1 - F0) * pow(2, (−5.55473(v·h) − 6.98316)(v·h))
 *      with
 *        F0 = specular reflectance color at normal incidence (constant of 0.04 for nonmetals,
 *        tinted by base albedo for metals)
 * 
 *    Denominator 4 * (n dot l) * (n dot v) is a correction factor, accounts for quantities
 *    being transformed between the local space of the microgeometry and the overall macrosurface
 * 
 ******************************************************************************/

float trowbridgeReitzNDF(float NdotH, float roughness) {
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float NdotH2 = NdotH * NdotH;
  float denom = pi * std::pow(NdotH2 * (alpha2 - 1.0f) + 1.0f, 2.0f);

  return alpha2 / denom;
}

//float trowbridgeReitzNDFAniso()

float schlickGAF(float NdotV, float roughness) {
  float k = ((roughness + 1.0f) * (roughness + 1.0f)) / 8.0f;

  return NdotV / (NdotV * (1.0f - k) + k);
}

inline vec3f fresnelEpic(const vec3f& F0, float HdotV) {
  float power = pow(2.0f, (-5.55473f * HdotV - 6.98316f) * HdotV);
  
  return vec3f(F0(0) + (1.0f - F0(0)) * power,
                F0(1) + (1.0f - F0(1)) * power,
                F0(2) + (1.0f - F0(2)) * power);
}

#endif
