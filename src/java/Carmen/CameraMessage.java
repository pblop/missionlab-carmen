package carmen;


/**  Carmen CameraHandler's message
   */

public class CameraMessage extends Message {
  /** unit is pixels */
  public int _01_width;
  /** unit is pixels */
  public int _02_height;
  public int _03_bytes_per_pixel;
  /** size of image[]  */
  public int _04_image_size;
  /** camera image */
  public char _05_image[];

  private static final String CARMEN_CAMERA_IMAGE_NAME = 
    "carmen_camera_image";    
  private static final String CARMEN_CAMERA_IMAGE_FMT = 
    "{int,int,int,int,<char:4>,double,string}";

  public static void subscribe(CameraHandler handler) {
    subscribe(CARMEN_CAMERA_IMAGE_NAME, CARMEN_CAMERA_IMAGE_FMT, handler, 
	      CameraMessage.class, "handle");
  }
  
  public static void unsubscribe() {
	    unsubscribe(CARMEN_CAMERA_IMAGE_NAME);
	  }

  public void publish() {
    publish(CARMEN_CAMERA_IMAGE_NAME, CARMEN_CAMERA_IMAGE_FMT, this);
  }
}

