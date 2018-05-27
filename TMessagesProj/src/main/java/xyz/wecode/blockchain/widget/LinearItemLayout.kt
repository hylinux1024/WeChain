package xyz.wecode.blockchain.widget

import android.annotation.TargetApi
import android.content.Context
import android.util.AttributeSet
import android.widget.ImageView
import android.widget.RelativeLayout
import android.widget.TextView
import org.jetbrains.anko.find
import org.telegram.messenger.R

class LinearItemLayout : RelativeLayout {

    val itemIcon by lazy { find<ImageView>(R.id.itemIcon) }
    val itemTitle by lazy { find<TextView>(R.id.itemTxt) }
    val itemRightText by lazy { find<TextView>(R.id.itemRightTxt) }

    constructor(context: Context) : super(context) {
        init(null, 0, 0)
    }

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        init(attrs, 0, 0)
    }

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        init(attrs, defStyleAttr, 0)
    }

    @TargetApi(21)
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int, defStyleRes: Int) : super(context, attrs, defStyleAttr, defStyleRes) {
        init(attrs, defStyleAttr, defStyleRes)
    }

    private fun init(attrs: AttributeSet?, defStyleAttr: Int, defStyleRes: Int) {
        inflate(context, R.layout.include_linear_item, this)
        val a = context.obtainStyledAttributes(attrs, R.styleable.LinearItemLayout, defStyleAttr, defStyleRes)
        val title = a.getResourceId(R.styleable.LinearItemLayout_itemTitle, 0)
        if (title > 0) {
            itemTitle.text = context.getString(title)
        }
        val icon = a.getResourceId(R.styleable.LinearItemLayout_itemIcon, 0)
        if (icon > 0) {
            itemIcon.setImageResource(icon)
        }
        val rightText = a.getResourceId(R.styleable.LinearItemLayout_itemRightText, 0)
        if (rightText > 0) {
            itemRightText.text = context.getString(rightText)
        }
        a.recycle()
        isClickable = true
    }


}