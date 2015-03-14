# Release name
PRODUCT_RELEASE_NAME := n5100

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Inherit device configuration
$(call inherit-product, device/samsung/n5100/full_n5100.mk)

# Device identifier. This must come after all inclusions
PRODUCT_DEVICE := n5100
PRODUCT_NAME := cm_n5100
PRODUCT_BRAND := samsung
PRODUCT_MODEL := GT-N5100
PRODUCT_MANUFACTURER := samsung

# Set build fingerprint / ID / Product Name ect.
PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=GT-N5100 TARGET_DEVICE=GT-N5100 BUILD_FINGERPRINT="samsung/kona3gxx/kona3g:4.3/JSS15J/N5100XXBMD1:user/release-keys" PRIVATE_BUILD_DESC="kona3gxx-user 4.3 JSS15J N5100XXBMD1 release-keys"
