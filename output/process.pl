#!/usr/bin/env perl6

sub MAIN(Str $folder) {
    if $folder eq 'test' {
	run_tests;
	exit
    }
    my $foldername = $folder;
    $foldername = "{$folder}/" unless $folder ~~ /\/$/;
    my @files = $foldername.IO.dir;
    my @times = ();
    my @samples = gather for @files -> $file {
	next if $file.path ~~ /summary/;
	my @positions = $file.slurp.chomp.split("\n").map: *.split(",");
	@times.push(@positions.pop[0]);
	take @positions;# if -10 < @positions[0][2] < 10;
    }

    my $t_mean = mean(@times);
    my $t_dev = dev(@times, $t_mean);
    my @summaries = map &mean_and_std, ([Z] @samples);

    my @output = @summaries.map: -> @list { @list.flat.join(",") };
    @output.push("iterations,{@samples.elems}");
    @output.push("time,{$t_mean},{$t_dev}\n");
    spurt "{$foldername}summary.csv", @output.join("\n");
}

sub mean(@l) {
    @l R/ [+] @l;
}

sub dev(@l, $mean) {
    sqrt (@l - 1) R/ [+] map (* - $mean)**2, @l;
}

sub mean_and_std(@position) {
    my @zipped = [Z] @position;
    my @means = map &mean, @zipped;
    my @stddevs = @zipped.kv.map: -> $i, @l {
	my $mean = @means[$i];
	dev(@l, $mean)
    };
    (@means, @stddevs);
}

sub run_tests {
    use Test;
    my @case = ((1,2,3,4,5,6,7,8,9,10), (2,3,4,5,6,17,8,9,10,11), (3,4,14,6,7,8,9,10,11,12));
    my @expected = (5.5, 7.5, 8.4, 3.0276503541, 4.503085362, 3.5023801431);
    my @results = mean_and_std([Z] @case).flat;
    for @expected.kv -> $i, $v {
	ok (@results[$i] - $v).abs < 0.0000001, "position $i"
    }
}
