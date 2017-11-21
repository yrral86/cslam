#!/usr/bin/env perl6

my $original_template = 'const.h.template'.IO.slurp;

my @particle_counts = (1000, 2000, 3000, 4000);
my @culling_factors = (1);
my $iterations = 100;

for @particle_counts -> $particle_count {
    for @culling_factors -> $culling_factor {
	my $initial_particle_factor = (40000/$particle_count).ceiling;

	my $template = $original_template;
	$template .= subst(/particle_count/, $particle_count.Str);
	$template .= subst(/initial_particle_factor/, $initial_particle_factor.Str);
	$template .= subst(/culling_factor/, $culling_factor.Str);

	spurt 'const.h', $template;

	shell 'make';
	my $dir = "output/{$particle_count}-{$culling_factor}";
	mkdir $dir;

	for 1..$iterations -> $iteration {
	    ($particle_count, $culling_factor, $iteration).join("-").say;
	    shell "./replay $dir/run-{$iteration}.csv"
	}

	shell "./output/process.pl $dir";
    }
}
