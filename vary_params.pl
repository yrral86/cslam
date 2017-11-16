#!/usr/bin/env perl6

my $template = 'const.h.template'.IO.slurp;

my $particle_count = 20000;
my $initial_particle_factor = (20000/$particle_count).ceiling;
my $culling_factor = 50;

$template .= subst(/particle_count/, $particle_count.Str);
$template .= subst(/initial_particle_factor/, $initial_particle_factor.Str);
$template .= subst(/culling_factor/, $culling_factor.Str);

spurt 'const.h', $template;

shell 'make';
my $dir = "output/{$particle_count}-{$culling_factor}";
mkdir $dir;

for 1..5 -> $iteration {
    shell "./replay $dir/run-{$iteration}.csv"
}

chdir 'output';
shell "./process.pl $dir";
chdir '..';
