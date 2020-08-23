package jp.araobp.amg8833

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import jp.araobp.amg8833.analyzer.RawImage
import jp.araobp.amg8833.serial.Amg8833Data
import jp.araobp.amg8833.serial.Amg8833Interface
import jp.araobp.amg8833.serial.IDataReceiver
import jp.araobp.amg8833.visualizer.R
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private var baudrate: Int = 9600
    private lateinit var amg8833Interface: Amg8833Interface

    private lateinit var rawImage: RawImage

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        rawImage = RawImage(surfaceView)

        amg8833Interface = Amg8833Interface(this, baudrate,
            object: IDataReceiver {
                override fun onAmg8833Data(amg8833Data: Amg8833Data) {
                    rawImage.draw(amg8833Data.data)
                }
            }
        )
        
    }
}