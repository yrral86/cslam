using NUnit.Framework;
using System;

namespace Slam
{
	[TestFixture ()]
	public class SlamTestClass
	{
		[Test ()]
		public void Test01Init ()
		{
			Swarm.swarm_init (681, 240, 7380, 3880, 1500);
		}

		[Test ()]
		public void Test02Move ()
		{
			Swarm.swarm_init (681, 240, 7380, 3880, 1500);
			Swarm.swarm_move (0, 0, 0);
		}

		[Test ()]
		public void Test03All ()
		{
			int[] distances;
			distances = new int[3];
			Swarm.swarm_init (3, 180, 7380, 3880, 1500);
			Swarm.swarm_move (0, 0, 0);
			// left
			distances [0] = 3 * 3880 / 4;
			// forward
			distances [1] = 7380 - 1500 / 2;
			//right
			distances [2] = 3880 / 4;
			Swarm.swarm_update (ref distances);
			// within 5 mm
			Assert.IsTrue (Swarm.swarm_get_best_x () - 1500 / 2 < 5);
			Assert.IsTrue (Swarm.swarm_get_best_y () - 3880 / 4 < 5);
			// within 5 degrees
			Assert.IsTrue (Swarm.swarm_get_best_theta () - 0 < 5);
		}
	}
}

