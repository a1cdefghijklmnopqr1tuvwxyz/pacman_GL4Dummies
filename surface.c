/*!\file surface.c 
 *
 * \brief gestion de surfaces et autres éléments de la scène :
 * lumière(s) (TODO), options, textures ...
 *
 * \author Farès BELHADJ, amsi@up8.edu
 * \date November 17, 2021. 
*/

#include "rasterize.h"
#include <assert.h>

/*!\brief calcule le vecteur normal à un triangle */
void tnormal(triangle_t * t) {
  vec3 u = {
	    t->v[1].position.x - t->v[0].position.x,
	    t->v[1].position.y - t->v[0].position.y,
	    t->v[1].position.z - t->v[0].position.z
  };
  vec3 v = {
	    t->v[2].position.x - t->v[0].position.x,
	    t->v[2].position.y - t->v[0].position.y,
	    t->v[2].position.z - t->v[0].position.z
  };
  MVEC3CROSS((float *)&(t->normal), (float *)&u, (float *)&v);
  MVEC3NORMALIZE((float *)&(t->normal));
}

/*!\brief calcule les vecteurs normaux aux triangles de la surface */
void snormals(surface_t * s) {
  int i;
  for(i = 0; i < s->n; ++i)
    tnormal(&(s->t[i]));
}

/*!\brief affecte les normales aux triangles de la surface à ses vertices */
void tnormals2vertices(surface_t * s) {
  int i;
  for(i = 0; i < s->n; ++i)
    s->t[i].v[0].normal = s->t[i].v[1].normal = s->t[i].v[2].normal = s->t[i].normal;
}

/*!\brief affecte l'identifiant de texture de la surface */
void set_texture_id(surface_t * s, GLuint tex_id) {
  s->tex_id = tex_id;
}

/*!\brief affecte la couleur diffuse de la surface */
void set_diffuse_color(surface_t * s, vec4 dcolor) {
  s->dcolor = dcolor;
}

/*!\brief active une option de la surface */
void enable_surface_option(surface_t * s, soptions_t option) {
  if(!(s->options & option))
    s->options |= option;
  updatesfuncs(s);
}

/*!\brief désactive une option de la surface */
void disable_surface_option(surface_t * s, soptions_t option) {
  if(s->options & option)
    s->options ^= option;
  updatesfuncs(s);
}

/*!\brief créé et renvoie une surface (allouée) à partir de \a n
 * triangles pointés par \a t. Quand \a duplicateTriangles est vrai
 * (1), elle alloue de la mémoire pour copier les triangles dedans,
 * sinon ( si faux (0) ) elle se contente de copier le pointeur
 * (attention ce dernier doit donc correspondre à une mémoire allouée
 * avec malloc et dont le développeur ne s'en servira pas pour autre
 * chose ; elle sera libérée par freeSurface). Quand \a hasNormals est
 * faux (0) elle force le calcul des normales par triangle et les
 * affecte aux sommets. */
surface_t * new_surface(triangle_t * t, int n, int duplicate_triangles, int has_normals) {
  const vec4 dcolor = { 0.42f, 0.1f, 0.1f, 1.0f };
  surface_t * s = malloc(1 * sizeof *s);
  assert(s);
  s->n = n;
  if(duplicate_triangles) {
    s->t = malloc(s->n * sizeof *(s->t));
    assert(s->t);
    memcpy(s->t, t, s->n * sizeof *(s->t));
  } else
    s->t = t;
  set_diffuse_color(s, dcolor);
  s->options = SO_DEFAULT;
  s->tex_id = 0;
  updatesfuncs(s);
  if(!has_normals) {
    snormals(s);
    tnormals2vertices(s);
  }
  return s;
}

/*!\brief libère la mémoire utilisée par la surface */
void free_surface(surface_t * s) {
  free(s->t);
  free(s);
}
/*!\brief charge et fabrique un identifiant pour une texture issue
 * d'un fichier BMP */
GLuint get_texture_from_BMP(const char * filename) {
  GLuint id, old_id;
  /* chargement d'une image dans une surface SDL */
  SDL_Surface * s = SDL_LoadBMP(filename);
  assert(s);
  old_id = gl4dpGetTextureId(); /* au cas où */
  /* création d'un screen GL4Dummies aux dimensions de la texture */
  id = gl4dpInitScreenWithDimensions(s->w, s->h);
  /* copie de la surface SDL vers le screen en cours */
  {
    GLuint * p = gl4dpGetPixels();
    SDL_Surface * d = SDL_CreateRGBSurface(0, s->w, s->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
    SDL_BlitSurface(s, NULL, d, NULL);  
    memcpy(p, d->pixels, d->w * d->h * sizeof *p);
    SDL_FreeSurface(d);
  }
  /* libération de la surface SDL */
  SDL_FreeSurface(s);
  if(old_id)
    gl4dpSetScreen(old_id);
  return id;
}
