package com.embeddedartists.aoa;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.android.future.usb.UsbAccessory;
import com.android.future.usb.UsbManager;
import com.embeddedartists.aoa.AccessoryControl.OpenStatus;

public class MainActivity extends Activity implements OnClickListener {
	
	/**************************************************************************
	 * Private variables 
	 *************************************************************************/

	/** UI components. They are declared in the XML layout (main.xml) */
	private TextView tempArea;
	private ImageView joyImgView;
	private LinearLayout topLayout;
	
	private AccessoryControl accessoryControl;
	
	/** TAG used for logging messages */
	private static final String TAG = "EA AOADemo";

	
	/**************************************************************************
	 * Overridden methods 
	 *************************************************************************/	
		
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        Log.i(TAG, "onCreate");
        
        setContentView(R.layout.main);
        
        ((Button)findViewById(R.id.redBtn)).setOnClickListener(this);
        ((Button)findViewById(R.id.greenBtn)).setOnClickListener(this);
        ((Button)findViewById(R.id.blueBtn)).setOnClickListener(this);    
                                 
        tempArea = (TextView)findViewById(R.id.tempValue);
        joyImgView = (ImageView)findViewById(R.id.joyImageView);
                
        accessoryControl = new AccessoryControl(this, handler);
        
        topLayout = (LinearLayout) findViewById(R.id.topLayout);
        topLayout.setBackgroundColor(Color.BLACK);        
        
        /*
         * Register a receiver for permission (granted/not granted)
         * messages and Accessory detached messages.
         */
        
        IntentFilter filter = new IntentFilter(AccessoryControl.ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
        registerReceiver(receiver, filter);		  
        
    }

		
	@Override
	protected void onResume() {
		super.onResume();
		
		Log.i(TAG, "onResume");
		
		/*
		 * The application has been resumed. Try to open
		 * and access the accessory.
		 */		
		
		disconnected();

		OpenStatus status = accessoryControl.open();
		if (status == OpenStatus.CONNECTED) {
			connected();
		}
		else if(status != OpenStatus.REQUESTING_PERMISSION 
				&& status != OpenStatus.NO_ACCESSORY) {
			showError(status);
		}
					
	}	

	@Override
	protected void onPause() {

		Log.i(TAG, "onPause");	
		
		/*
		 * The application is about to be paused/closed. Make sure
		 * the accessory gets a notification about this.
		 */
		
		accessoryControl.appIsClosing();				
		accessoryControl.close();
		super.onPause();
	
	}	
	
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		
		Log.i(TAG, "onDestroy");		
		
		/*
		 * The application is completely closed and removed from memory.
		 * Remove all resources which in this case means unregistering the
		 * broadcast receiver.
		 */
		
		unregisterReceiver(receiver);
	}
	
	/**
	 * Click Handler which is invoked when clicking on a 
	 * UI button. 
	 */
	public void onClick(View v) {
		
		switch (v.getId()) {
		case R.id.redBtn:

			accessoryControl.writeCommand(
					AccessoryControl.MESSAGE_OUT_RGB_LED, 
					AccessoryControl.MESSAGE_RGB_VAL_RED, 
					(((ToggleButton)v).isChecked() ? 1 : 0));				
			break;
		case R.id.greenBtn:

			accessoryControl.writeCommand(
					AccessoryControl.MESSAGE_OUT_RGB_LED, 
					AccessoryControl.MESSAGE_RGB_VAL_GREEN, 
					(((ToggleButton)v).isChecked() ? 1 : 0));			
			break;
		case R.id.blueBtn:

			accessoryControl.writeCommand(
					AccessoryControl.MESSAGE_OUT_RGB_LED, 
					AccessoryControl.MESSAGE_RGB_VAL_BLUE, 
					(((ToggleButton)v).isChecked() ? 1 : 0));			
			break;
		}
		
	}
	
	/**************************************************************************
	 * Private methods  
	 *************************************************************************/
	
	/**
	 * Indicate in the UI that an accessory is connected.
	 */
	private void connected() {
		setTitle("Accessory Connected");
		topLayout.setBackgroundColor(Color.rgb(0, 50, 0));
	}
	
	/**
	 * Indicate in the UI that an accessory isn't connected.
	 */	
	private void disconnected() {
		setTitle("Accessory Disconnected");	
		topLayout.setBackgroundColor(Color.BLACK);
	}
	
	/**
	 * Show an error dialog as a response to opening an accessory.
	 */	
	private void showError(OpenStatus status) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setMessage("Error: " + status.toString());

		AlertDialog d = builder.create();
		d.show();
	}
	
	/**************************************************************************
	 * Anonymous classes 
	 *************************************************************************/	

	/*
	 * The Android UI Toolkit is not thread-safe which means that only the 
	 * UI Thread may manipulate the User Interface. 
	 * 
	 * A Handler allows a thread to send a message to another thread's 
	 * message queue. When the Handler is created it will be associated
	 * with the thread that is creating the Handler. For the Handler below
	 * this means the main (UI) thread.
	 * 
	 * Since messages from the Accessory are received by a separate thread
	 * those messages must be sent to the Handler below which is responsible
	 * for updating the UI.
	 */
	
	private final Handler handler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			
			switch(msg.what) {
			case AccessoryControl.MESSAGE_IN_TEMPERATURE: 
				tempArea.setText("" + (double)msg.arg1 / 10 + "º");
				break;
			case AccessoryControl.MESSAGE_IN_JOYSTICK:
				switch(msg.arg1) {
				case 0:
					joyImgView.setImageResource(R.drawable.joy_default);
					break;
				case 1:
					joyImgView.setImageResource(R.drawable.joy_down);
					break;
				case 257:
					joyImgView.setImageResource(R.drawable.joy_up);
					break;
				case 513:
					joyImgView.setImageResource(R.drawable.joy_right);
					break;					
				case 769:
					joyImgView.setImageResource(R.drawable.joy_left);
					break;					
											
				}
		
				break;
			}
		}
		
	};
	

	/*
	 * A fundamental part of Android applications is intents. Intents are 
	 * messages between components and applications. 
	 * 
	 * The Broadcast receiver handles broadcast intents and the receiver
	 * below has been register in onCreate above with a filter to receive
	 * Accessory Detached and permission actions.
	 */
	
	private final BroadcastReceiver receiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {

			String action = intent.getAction();
			if (AccessoryControl.ACTION_USB_PERMISSION.equals(action)) {
				UsbAccessory accessory = UsbManager.getAccessory(intent);
				
				if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {	

					Log.i(TAG, "Permission Granted");
					OpenStatus status = accessoryControl.open(accessory);
					if (status == OpenStatus.CONNECTED) {
						connected();
					}
					else {
						showError(status);
					}
				}
				else {
					Log.i(TAG, "Permission NOT Granted");
					disconnected();
				}
				
				
			} else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {		
				Log.i(TAG, "Detached");
				disconnected();				
				accessoryControl.close();
			}
			
			
		}
		
	};		

}