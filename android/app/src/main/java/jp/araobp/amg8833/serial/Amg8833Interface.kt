package jp.araobp.amg8833.serial

import android.content.Context
import android.util.Log
import jp.araobp.uart.UsbSerial

class Amg8833Interface(context: Context, baudrate: Int, val receiver: IDataReceiver) :
    UsbSerial(context, baudrate, 8, 8, 0, 0) {

    companion object {
        val TAG: String = this::class.java.simpleName

        val BEGIN = 0xFE.toByte()
        val END = 0xFFU.toByte()
    }

    enum class StateMachine {
        BEGIN_WAITING,
        DATA_RECEIVING
    }

    var state = StateMachine.BEGIN_WAITING

    val data = UByteArray(64)

    var idx = 0

    override fun parse(messageFraction: ByteArray, len: Int) {
        for (i in 0 until len) {
            val b = messageFraction[i]

            if (state == StateMachine.BEGIN_WAITING && b == BEGIN) {
                state = StateMachine.DATA_RECEIVING
            } else if (state == StateMachine.DATA_RECEIVING) {
                when (idx < 64) {
                    true -> {
                        data[idx++] = b.toUByte()
                    }
                    false -> {
                        if (b == END) {
                            receiver.onAmg8833Data(Amg8833Data(data))
                        } else {
                            Log.d(TAG, "Out of sync")
                        }
                        state = StateMachine.BEGIN_WAITING
                        idx = 0
                    }
                }
            }
        }
    }
}
