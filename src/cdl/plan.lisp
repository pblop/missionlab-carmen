;;/*************************************************
;;*
;;* This file plan.lisp was created with the command
;;* cdl test_ugv.cdl
;;* using the CDL compiler, version 1.0
;;*
;;**************************************************/

;; =====================================
;; SAUSAGES plan robot for test_ugvRobot1
;; =====================================
(setq start-loc '(1425.0 675.0))

(defplan the-plan ()
  (sequence

    ;;Start
    (link user-wait
          (plan-id 0)
          (position 
            '(1425.0 675.0))
          (message "Waiting for proceed message")
    )

    ;;$AN_262
    (link xcountry
          (plan-id 1)
          (initial-speed 4.0)
          (points (vector
            '(1425.0 675.0)
            '(900.000000 450.000000)))
    )

    ;;$AN_266
    (link user-wait
          (plan-id 2)
          (position 
            '(900.000000 450.000000))
          (message "Waiting for proceed message")
    )
  )
)

