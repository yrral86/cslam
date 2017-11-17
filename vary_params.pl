#!/usr/bin/env perl6

my $template = 'const.h.template'.IO.slurp;

#my @particle_counts = (5000, 10000, 20000, 40000);
#my @culling_factors = (1, 25, 50, 100);
my @particle_counts = (20000);
my @culling_factors = (50);
my $iterations = 5;

for @paricle_counts -> $particle_count {
    for @culling_factors -> $culling_factor {
	my $initial_particle_factor = (40000/$particle_count).ceiling;

	$template .= subst(/particle_count/, $particle_count.Str);
	$template .= subst(/initial_particle_factor/, $initial_particle_factor.Str);
	$template .= subst(/culling_factor/, $culling_factor.Str);

	spurt 'const.h', $template;

	shell 'make';
	my $dir = "output/{$particle_count}-{$culling_factor}";
	mkdir $dir;

	for 1..$iterations -> $iteration {
	    shell "./replay $dir/run-{$iteration}.csv"
	}

	chdir 'output';
	shell "./process.pl $dir";
	chdir '..';
    }
}
