#include "evitar_obstaculos.h"


Vector evitar_obstaculos(){
    double  sphere;
    double   safety_margin = 0.20;
    obs_array    readings;
    Vector   repuls;     /* vector along direction of repulsion
    for a specific obstacle */
    int      i;      /* current obstacle */
    double   c_to_c_dist;    /* distance from center of robot to center
    of obstacle */
    double       mag;        /* desired magnatude of a repulsion vector
    (repuls) for a specific obstacle */
    double       radius;     /* radius of current obstacle + safety_margin */
    int          first_time=TRUE;
    double       old_safety_margin = 0;
    Vector output;

//     fprintf(stderr,"\nevitar Obstaculos: Principio\n");
    readings = exec_detect_obstacles_mio(1000);
//     fprintf(stderr,"\nevitar Obstaculos: despues de evitar obstaculos\n");

    if( first_time || old_safety_margin != safety_margin )
    {
//         fprintf(stderr,"\nevitar Obstaculos: dentro del if\n");
        /*
        * Report the safety margin we are using to the console so the
        * graphics can use it for showing virtual collisions
        */
        char buf[80];

        sprintf(buf, "%f",safety_margin);
//         fprintf(stderr,"\nevitar Obstaculos: despues sprintf\n");
        exec_put_console_state(SAFETY_MARGIN_MSG,buf);
//         fprintf(stderr,"\nevitar Obstaculos: despues exec_put_console_state\n");

        old_safety_margin = safety_margin;
        first_time = FALSE;
    }

    VECTOR_CLEAR(output);

    /* readings.size is the number of obstacles
    readings.val is a Vector to the center of the obstacle
    so readings.val.x and readings.val.y are the center numbers.
    readings.r is a corresponding array of each obstacle's radius.
    */

//     fprintf(stderr,"\nevitar Obstaculos: despues VECTOR_CLEAR\n");

    for(i=0; i<readings.size; i++)
    {
        /* compute the distance from the center of the robot to the
        center of the obstacle in meters
        */
        c_to_c_dist = len_2d(readings.val[i].center);
        radius = readings.val[i].r + safety_margin;


        /* Handle the case where within the obstacle */
        if (c_to_c_dist <= radius )
        {
            /* generate an infinite (around 100000) vector away from the obstacle*/
            mag = GT_INFINITY;

            if (c_to_c_dist < EPS_ZERO) /* Epsilon Zero (around 0.00001) */
            {
            /* Handle the case where an obstacle is EXACTLY centered
                where the robot is as a special case because we can't
                use the vector to determine a direction to move.
                So, arbitraily choose positive Y direction for vector.
            */

                readings.val[i].center.y = 1.0;
            }
        }

        /* handle case where within the linear repulsion zone */
        else if (c_to_c_dist <= radius + sphere )
        {
            /* set the magnitude of the repulsion vector (0...1) based on how
            far we have intruded into the zone.
            Magnitude is 0 on outside edge (c_to_c_dist = sphere + radius).
            Magnitude is 1 on inside edge (c_to_c_dist = radius)

            sphere is the size of the zone.
            We are at distance (c_to_c_dist - radius) from the inner edge of
            the zone or (sphere - (c_to_c_dist - radius)) from the outer edge.

            So, take our distance from the outer edge and divide by the size
            of the zone to get a ratio from 0 to 1 with zero at the outside
            and 1 at the inside.
            */

            mag = (sphere - (c_to_c_dist - radius)) / sphere;
        }

        /* otherwise, outside obstacle's sphere of influence, so ignore it */
        else
        {
            mag = 0;
        }

        /* if needed, generate a repulsive vector and add it to the running sum */
        if (mag != 0)
        {
            /* create a unit vector along the direction of repulsion */
            repuls.x = -readings.val[i].center.x;
            repuls.y = -readings.val[i].center.y;
            unit_2d(repuls);

            /* Set its strength to the value selected */
            mult_2d(repuls, mag);

            /* Add it to the running sum */
            plus_2d(output, repuls);

            if( debug )
            {
                fprintf(stderr,"evitar Obstaculos: <%.1f %.1f>, %.1f => repulsive vector <%.1f %.1f>\n",
                        readings.val[i].center.x,
                        readings.val[i].center.y,
                        readings.val[i].r,
                        repuls.x, repuls.y);
            }

        }
    }

    if( debug )
    {
        fprintf(stderr,"evitar Obstaculos: output vector <%.1f %.1f>\n",
                output.x, output.y);
    }
    //fprintf(stderr,"\nevitar Obstaculos: output vector <%.1f %.1f>\n", output.x, output.y);
    return output;
}
