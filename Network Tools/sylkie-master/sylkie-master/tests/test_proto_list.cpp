#include <gtest/gtest.h>
#include <iostream>

extern "C" {
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <netinet/ip6.h>
#include <proto_list.h>
#include <strings.h>
}

TEST(proto_list, basic) {
    struct icmp6_hdr icmp6;
    struct sylkie_proto_list* lst = sylkie_proto_list_init();
    enum sylkie_error err;

    bzero(&icmp6, sizeof(struct icmp6_hdr));
    icmp6.icmp6_type = ND_NEIGHBOR_ADVERT;
    icmp6.icmp6_data8[0] = 0x20;
    err = sylkie_proto_list_add(lst, SYLKIE_ICMPv6, &icmp6,
                                sizeof(struct icmp6_hdr));
    ASSERT_EQ(err, SYLKIE_SUCCESS);
    sylkie_proto_list_free(lst);
}

TEST(proto_list, rm) {
    int i = 0;
    struct ethhdr eth;
    struct ip6_hdr ipv6;
    struct icmp6_hdr icmp6;
    struct sylkie_proto_node* node;
    struct sylkie_proto_list* lst = sylkie_proto_list_init();
    enum sylkie_error err;

    err = sylkie_proto_list_add(lst, SYLKIE_ETH, &eth, sizeof(struct ethhdr));
    ASSERT_EQ(err, SYLKIE_SUCCESS);

    err =
        sylkie_proto_list_add(lst, SYLKIE_IPv6, &ipv6, sizeof(struct ip6_hdr));
    ASSERT_EQ(err, SYLKIE_SUCCESS);

    err = sylkie_proto_list_add(lst, SYLKIE_ICMPv6, &icmp6,
                                sizeof(struct icmp6_hdr));
    ASSERT_EQ(err, SYLKIE_SUCCESS);

    err = sylkie_proto_list_add(lst, SYLKIE_DATA, NULL, 0);
    ASSERT_EQ(err, SYLKIE_SUCCESS);

    SYLKIE_HEADER_LIST_FOREACH(lst, node) {
        switch (i) {
        case 0:
            EXPECT_EQ(node->hdr.type, SYLKIE_ETH);
            break;
        case 1:
            EXPECT_EQ(node->hdr.type, SYLKIE_IPv6);
            break;
        case 2:
            EXPECT_EQ(node->hdr.type, SYLKIE_ICMPv6);
            break;
        case 3:
            EXPECT_EQ(node->hdr.type, SYLKIE_DATA);
            break;
        default:
            ASSERT_TRUE(false);
            break;
        }
        ++i;
    }

    ASSERT_EQ(i, 4);

    err = sylkie_proto_list_rm(lst, SYLKIE_IPv6);
    ASSERT_EQ(err, SYLKIE_SUCCESS);

    i = 0;
    SYLKIE_HEADER_LIST_FOREACH(lst, node) {
        switch (i) {
        case 0:
            EXPECT_EQ(node->hdr.type, SYLKIE_ETH);
            break;
        case 1:
            EXPECT_EQ(node->hdr.type, SYLKIE_ICMPv6);
            break;
        case 2:
            EXPECT_EQ(node->hdr.type, SYLKIE_DATA);
            break;
        default:
            ASSERT_TRUE(false);
            break;
        }
        ++i;
    }

    ASSERT_EQ(i, 3);

    sylkie_proto_list_free(lst);
}
