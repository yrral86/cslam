#!/usr/bin/env perl6

use Chart::Gnuplot;

my @results = "results.csv".IO.slurp.chomp.split("\n")>>.split(",").map: -> @row {
    %(
	 'particle-count' => @row[0].Int,
	 'culling-factor' => @row[1].Int,
	 'positional-deviation' => @row[2].Rat,
	 'angular-deviation' => @row[3].Rat,
	 'time' => @row[4].Rat/1000
    )
};

my $runtime_chart = start_chart('runtime', 'Runtime', 'Particle Count', 'Time (s)');
for 1, 5, 20 -> $factor {
    my @selected = @results.grep: *{'culling-factor'} == $factor;
    plot_one($runtime_chart, @selected, "Culling Factor $factor", 'particle-count', 'time')
}
finish_chart($runtime_chart);

for 1000, 8000, 64000 -> $count {
    my $culling_runtime_chart = start_chart("culling-{$count}-runtime",
					    "{$count/1000}K Particles Runtime",
					    'Culling Factor', 'Time (s)', True);
    my @data = @results.grep: -> %row {
	%row{'particle-count'} == $count &&
	    ((%row{'culling-factor'} != 160 && $count != 64000) ||
	     %row{'culling-factor'} < 21)
    };
    plot_one($culling_runtime_chart, @data, '', 'culling-factor', 'time');
    finish_chart($culling_runtime_chart)
}


for ('Positional', 'mm', 'Angular', 'degrees') -> $type, $units {
    my $accuracy_chart = start_chart("accuracy-{$type.lc}", "$type Deviation vs Particle Count",
				 'Particle Count', "$type Deviation ($units)");
    for 1, 5, 20 -> $factor {
	my @selected = @results.grep: *{'culling-factor'} == $factor;
	plot_one($accuracy_chart, @selected, "Culling Factor $factor",
		 'particle-count', "{$type.lc}-deviation")
    }
    finish_chart($accuracy_chart);

    my $factor_chart = start_chart("culling-accuracy-{$type.lc}", "$type Deviation vs Culling Factor",
				  "Culling Factor", "$type Deviation ($units)");
    for 1000, 8000, 64000 -> $count {
	my @selected = @results.grep: -> %row {
	    %row{'particle-count'} == $count &&
		((%row{'culling-factor'} < 160 && $count != 64000) ||
		 %row{'culling-factor'} < 21)
	};
	plot_one($factor_chart, @selected, "$count Particles",
		 'culling-factor', "{$type.lc}-deviation")
    }
    finish_chart($factor_chart)			     
}

sub start_chart(Str $filename, Str $title, Str $xLabel, Str $yLabel, Bool $noLegend = False) {
    my $gnu = Chart::Gnuplot.new(:terminal("png"), :filename("{$filename}.png"));
    $gnu.legend(:left) unless $noLegend;
    if $filename eq 'culling-accuracy-angular' {
	$gnu.yrange(:min(0), :max(120));
	$gnu.xrange(:min(0), :max(90))
    } elsif $filename eq 'culling-accuracy-positional' {
	$gnu.yrange(:min(0), :max(800));
	$gnu.xrange(:min(0), :max(90))
    } elsif $filename ~~ /culling\-.*\-runtime/ {
	if $filename eq 'culling-64000-runtime' {
	    $gnu.xrange(:min(0), :max(25))
	} else {
	    $gnu.xrange(:min(0), :max(90))
	}
    }
    $gnu.title(:text($title));
    $gnu.xlabel(:label($xLabel));
    $gnu.ylabel(:label($yLabel));
    $gnu
}

sub plot_one($chart, @data, Str $title, Str $xLabel, Str $yLabel) {
    $chart.plot(:title($title), :vertices(extract(@data, $xLabel, $yLabel)),
		:style('linespoints'), :ps(2.5));
}

sub finish_chart($chart) {
    $chart.dispose
}

sub extract(@rows, Str $xLabel, Str $yLabel) {
    @rows.map: -> %row {
	[%row{$xLabel}, %row{$yLabel}]
    }
}
