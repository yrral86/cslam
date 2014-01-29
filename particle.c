#include "particle.h"

void particle_add_sample(particle *p, double score) {
  // p->score is average score for p->samples
  // new p->score is average score for p->samples + 1
  // p->score * p->samples + score / (p->samples + 1)
  // weight new score higher by 75%
  p->score += (p->score*p->samples + 1.75*score)/(p->samples + 1);
  p->samples++;
}

particle particle_init(int x, int y, int theta) {
  particle p;
  p.x = x;
  p.y = y;
  p.theta = theta;
  p.samples = 0;
  p.score = 0;

  return p;
}
