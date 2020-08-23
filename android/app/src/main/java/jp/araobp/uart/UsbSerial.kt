package jp.araobp.uart

import android.content.Context
import android.util.Log
import com.ftdi.j2xx.D2xxManager
import com.ftdi.j2xx.FT_Device
import kotlin.concurrent.thread
import kotlin.experimental.or

/**
 * FTDI USB-Serial driver
 */
abstract class UsbSerial(val context: Context, baudrate: Int, dataBits: Byte, stopBits: Byte, parity: Byte, flowControl: Byte) {

    companion object {
        val TAG: String = this::class.java.simpleName
        const val READBUF_SIZE = 1024
    }

    private var mD2xxManager: D2xxManager? = null
    private var mFtdiDevice: FT_Device? = null

    private val mReadBuf = ByteArray(READBUF_SIZE)
    var mReaderThreadRunning = false

    init {
        while (true) {
            if(open(baudrate)) {
                setConfig(baudrate, dataBits, stopBits, parity, flowControl)
                break
            } else {
                Thread.sleep(1000)
            }
        }
    }

    /**
     * Opens FTDI device and start reader thread
     *
     * @parameter baudrate baud rate
     * @return true if FTDI device is opened successfully
     */
    private fun open(baudrate: Int): Boolean {
        var opened = false
        mD2xxManager = D2xxManager.getInstance(context)

        var devCount = mD2xxManager!!.createDeviceInfoList(context)
        Log.d(TAG, "Device number : " + Integer.toString(devCount))

        val deviceList = arrayOfNulls<D2xxManager.FtDeviceInfoListNode>(devCount)
        mD2xxManager!!.getDeviceInfoList(devCount, deviceList)

        if (devCount > 0) {
            mFtdiDevice = mD2xxManager!!.openByIndex(context, 0)
            mFtdiDevice?.let {
                if (it.isOpen) {
                    setConfig(baudrate, 8.toByte(), 1.toByte(), 0.toByte(), 0.toByte())
                    it.purge(D2xxManager.FT_PURGE_TX or D2xxManager.FT_PURGE_RX)
                    it.restartInTask()
                    mReaderThreadRunning = true

                    thread {
                        while (mReaderThreadRunning) {
                            var len = it.queueStatus
                            if (len > 0) {
                                if (len > READBUF_SIZE) {
                                    len = READBUF_SIZE
                                }
                                it.read(mReadBuf, len)
                                Log.d("FTDI", mReadBuf.toString())
                                _rx(mReadBuf, len)
                            }
                        }
                    }

                    opened = true
                }
            }
        }
        return opened
    }

    /**
     * Sets FTDI device config
     */
    private fun setConfig(baudrate: Int, dataBits: Byte, stopBits: Byte, parity: Byte, flowControl: Byte) {

        mFtdiDevice!!.setBitMode(0.toByte(), D2xxManager.FT_BITMODE_RESET)
        mFtdiDevice!!.setBaudRate(baudrate)

        val dataBitsByte: Byte = when (dataBits) {
            7.toByte() -> D2xxManager.FT_DATA_BITS_7
            8.toByte() -> D2xxManager.FT_DATA_BITS_8
            else -> D2xxManager.FT_DATA_BITS_8
        }

        val stopBitsByte: Byte = when (stopBits) {
            1.toByte() -> D2xxManager.FT_STOP_BITS_1
            2.toByte() -> D2xxManager.FT_STOP_BITS_2
            else -> D2xxManager.FT_STOP_BITS_1
        }

        val parityByte: Byte = when (parity) {
            0.toByte() -> D2xxManager.FT_PARITY_NONE
            1.toByte() -> D2xxManager.FT_PARITY_ODD
            2.toByte() -> D2xxManager.FT_PARITY_EVEN
            3.toByte() -> D2xxManager.FT_PARITY_MARK
            4.toByte() -> D2xxManager.FT_PARITY_SPACE
            else -> D2xxManager.FT_PARITY_NONE
        }
        mFtdiDevice!!.setDataCharacteristics(dataBitsByte, stopBitsByte, parityByte)

        val flowCtrlSetting: Short = when (flowControl) {
            0.toByte() -> D2xxManager.FT_FLOW_NONE
            1.toByte() -> D2xxManager.FT_FLOW_RTS_CTS
            2.toByte() -> D2xxManager.FT_FLOW_DTR_DSR
            3.toByte() -> D2xxManager.FT_FLOW_XON_XOFF
            else -> D2xxManager.FT_FLOW_NONE
        }
        mFtdiDevice!!.setFlowControl(flowCtrlSetting, 0x0b.toByte(), 0x0d.toByte())
    }

    /**
     * Transmits a message to FTDI device
     */
    fun tx(message: ByteArray) {
        mFtdiDevice?.let {
            if (it.isOpen) {
                it.latencyTimer = 16.toByte()
                it.write(message, message.size)
            } else {
                Log.e(TAG, "onClickWrite : device is not openDevice")
            }
        }
    }

    /**
     * Stops reader thread and closes FTDI device
     */
    fun destroy() {

        mReaderThreadRunning = false

        mFtdiDevice?.let {
            it.close()
        }
    }

    abstract fun parse(messageFraction: ByteArray, len: Int)

    private fun _rx(messageFraction: ByteArray, len: Int) {
        parse(messageFraction, len)
    }

}