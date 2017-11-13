#!/usr/bin/env perl6

sub MAIN(Str $folder) {
    my $foldername = $folder;
    $foldername = "{$folder}/" unless $folder ~~ /\/$/;
    my @files = $foldername.IO.dir;
    my @samples = gather for @files -> $file {
	next if $file.path ~~ /summary/;
	my @positions = $file.slurp.chomp.split("\n").map: *.split(",");
	take @positions if -10 < @positions[0][2] < 10;
    }

    my @summaries = map &mean_and_std, ([Z] @samples);

    my @output = @summaries.map: -> @list { @list.flat.join(",") };
    @output.push("iterations,{@samples.elems}\n");
    
    spurt "{$foldername}summary.csv", @output.join("\n");
}

sub mean_and_std(@position) {
    my @zipped = [Z] @position;
    my @means = @zipped.map: -> @l { @l R/ [+] @l };
    my @stddevs = @zipped.kv.map: -> $i, @l {
	my $mean = @means[$i];
	sqrt (@l - 1) R/ [+] map (* - $mean)**2, @l;
    };
    (@means, @stddevs);
}
