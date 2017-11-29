#!/usr/bin/env perl6

my $original_template = 'const.h.template'.IO.slurp;

my @particle_counts = (1000, 2000, 4000, 8000, 16000, 32000, 64000);
my @culling_factors = (1, 5, 10, 20);
my $iterations = 200;
my $iteration_offset = 0;

for @particle_counts -> $particle_count {
    for @culling_factors -> $culling_factor {
	my $initial_particle_factor = 1;

	my $template = $original_template;
	$template .= subst(/particle_count/, $particle_count.Str);
	$template .= subst(/initial_particle_factor/, $initial_particle_factor.Str);
	$template .= subst(/culling_factor/, $culling_factor.Str);

	spurt 'const.h', $template;

	shell 'make';
	my $dir = "output/{$particle_count}-{$culling_factor}";
	mkdir $dir;

	for (1+$iteration_offset)..($iterations+$iteration_offset) -> $iteration {
	    ($particle_count, $culling_factor, $iteration).join("-").say;
	    shell "./replay $dir/run-{$iteration}.csv"
	}

	shell "./output/process.pl $dir";
    }
}
