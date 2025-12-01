(defplan the-plan ()
  (sequence
   (link user-wait (message "Waiting for start mission") (plan-id 0)
         (position '(500.0 250.0)))

   (link xcountry 
         (points (vector '(500.0 250.0)
                         '(500.0 1000.0)
                         '(500.0 1900.0)))
         (initial-speed 3.0)
         (plan-id 4))

   (link user-wait (message "Done")
         (position '(500.0 1900.0))
         (plan-id 3))
  )
)
