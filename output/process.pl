#!/usr/bin/env perl6

sub MAIN(Str $folder) {
    my $foldername = $folder;
    $foldername = "{$folder}/" unless $folder ~~ /\/$/;
    my @files = $foldername.IO.dir;
    my @times = ();
    my @samples = gather for @files -> $file {
	next if $file.path ~~ /summary/;
	my @positions = $file.slurp.chomp.split("\n").map: *.split(",");
	@times.push(@positions.pop[0]);
	take @positions if -10 < @positions[0][2] < 10;
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
