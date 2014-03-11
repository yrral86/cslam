using NUnit.Framework;
using System;
using System.Linq;

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

		[Test ()]
		public void Test04Path ()
		{
			int x, y, dx, dy;
			int[] distances;
			distances = new int[3];
			x = 1500 / 2;
			y = 3880 / 4;
			dx = 20;
			dy = 20;
			Swarm.swarm_init (3, 180, 7380, 3880, 1500);
			Swarm.swarm_move (0, 0, 0);
			// left
			distances [0] = 3 * y;
			// forward
			distances [1] = 7380 - x;
			//right
			distances [2] = y;
			Swarm.swarm_update (ref distances);
			// within 5 mm
			Assert.IsTrue (Swarm.swarm_get_best_x () - x < 5);
			Assert.IsTrue (Swarm.swarm_get_best_y () - y < 5);
			for (int i = 0; i < 10; i++) {
				Swarm.swarm_move (dx, dy, 0);
				distances [0] -= dy;
				distances [1] -= dx;
				distances [2] += dy;
				Swarm.swarm_update (ref distances);
				x += dx;
				y += dy;
				Assert.IsTrue (Swarm.swarm_get_best_x () - x < 5);
				Assert.IsTrue (Swarm.swarm_get_best_y () - y < 5);
			}
		}
	}
}

