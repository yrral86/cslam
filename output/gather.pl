#!/usr/bin/env perl6

my @output = ();

for (dir) -> $dir {
    if $dir ~~ /\-/ {
	try {
	my ($count, $factor) = $dir.path.split('-');
	my @summary = "$dir/summary.csv".IO.slurp.chomp.split("\n").map: *.split(',');
	my $time = @summary.pop[1];
	@summary.pop;
	@summary = @summary.map: -> ($x, $y, $t, $x_sd, $y_sd, $t_sd) {
	    ($x_sd, $y_sd, $t_sd)
	}
	my @zipped = [Z] @summary;
	@zipped = (@zipped[0].flat, @zipped[1].flat).flat, @zipped[2];
	my @means = @zipped.map: -> @l {
	    @l R/ [+] @l
	};
	my @sd = @zipped.kv.map: -> $i, @l {
	    my $mean = @means[$i];
	    sqrt (@l - 1) R/ [+] map (* - $mean)**2, @l
	}
	
	@output.push("$count,$factor,@means[0],@means[1],$time");
	
    }
}

(@output.sort: -> $one, $two {
    my @one = $one.split(",");
    my @two = $two.split(",");
    if @one[0].Int == @two[0].Int {
	@one[1].Int > @two[1].Int
    } else {
	@one[0].Int > @two[0].Int
    }
 }).join("\n").say
